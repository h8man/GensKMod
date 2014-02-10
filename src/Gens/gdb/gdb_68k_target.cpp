#include "gdb_68k_target.h"

#include "../Star_68k.h"
#include "../Mem_M68K.h"
#include "../Mem_S68K.h"

#include "../vdp_io.h"

#include <string.h>


extern "C"
{
extern int Paused;
void __stdcall Sleep(unsigned int);
};

static inline unsigned int byteswap(unsigned int n)
{
    return (n >> 24) + ((n >> 8) & 0xFF00) + ((n << 8) & 0xFF0000) + (n << 24);
}

template <const bool is_main>
class gdb68KTarget : public gdbTarget
{
public:
    gdb68KTarget(void);
    ~gdb68KTarget(void);

    void Attach(gdbTargetController * controller)
    {
        S68000CONTEXT &context = is_main ? main68k_context : sub68k_context;

        m_controller = controller;

        context.resethandler = &reset_handler;

        if (is_main)
        {
            s_controller = controller;
            main68k_SetContext(&context);
        }
        else
        {
            s_controller = controller;
            sub68k_SetContext(&context);
        }
    }

    int GetRegisterCount()
    {
        return 4;
    }

    unsigned int GetRegisterValue(int index)
    {
        S68000CONTEXT &context = is_main ? main68k_context : sub68k_context;

        if (index < 0)
            return 0;

        if (index <= 7)
            return context.dreg[index];

        if (index <= 15)
            return context.areg[index - 8];

        if (index == 16)
            return context.sr;

        if (index == 17)
        {
            unsigned int value = is_main ? main68k_readPC() : sub68k_readPC();
            return s_broke ? value - 2 : value;
        }

        return 0;
    }

    void SetRegisterValue(int index, unsigned int value)
    {
        S68000CONTEXT &context = is_main ? main68k_context : sub68k_context;

        if (index < 0)
            return;

        if (index <= 7)
        {
            context.dreg[index] = value;
            return;
        }

        if (index <= 15)
        {
            context.areg[index - 8] = value;
            return;
        }

        if (index == 16)
        {
            context.sr = (unsigned short)value;
            return;
        }

        if (index == 17)
        {
            context.pc = value;
            return;
        }
    }

    void ReadMemory(unsigned int base, unsigned int count, void * data)
    {
        unsigned int n;
        unsigned char * d = (unsigned char *)data;

        if (is_main)
        {
            if (base > 0xFFFFFF)
            {
                if (base >= 0x1000000 && base < 0x1010000)
                {
                    memcpy(data, &VRam[base & 0xFFFF], count);
                    return;
                }
                if (base >= 0x1010000 && base < 0x1010100)
                {
                    base -= 0x1010000;
                    for (n = 0; n < count; n++)
                    {
                         d[n] = CRam[(base + n) ^ 1];
                    }
                    return;
                }
                if (base >= 0x1010100 && base < 0x1010200)
                {
                    base -= 0x1010100;
                    memcpy(data, &VSRam[base & 0xFFFF], count);
                    return;
                }
            }
        }

        if (is_main && (base < 6 * 1024 * 1024))
        {
            for (n = 0; n < count; n++)
            {
                *d++ = Rom_Data[(base + n) ^ 1];
            }
        }
        else
        {
            for (n = 0; n < count; n++)
            {
                *d++ = is_main ? M68K_RB(base + n) : S68K_RB(base + n);
            }
        }
    }

    void WriteMemory(unsigned int base, unsigned int count, const void * data)
    {
        unsigned int n;
        const unsigned char * d = (const unsigned char *)data;
        static const unsigned char bkpt[] = { 0x4E, 0x70 }; // { 0x48, 0x48 };

        if (is_main)
        {
            if (base > 0xFFFFFF)
            {
                if (base >= 0x1000000 && base < 0x1010000)
                {
                    memcpy(&VRam[base & 0xFFFF], data, count);
                    VRam_Flag = 1;
                    return;
                }
                if (base >= 0x1010000 && base < 0x1010100)
                {
                    base -= 0x1010000;
                    for (n = 0; n < count; n++)
                    {
                         CRam[(base + n) ^ 1] = d[n];
                    }
                    CRam_Flag = 1;
                    return;
                }
                if (base >= 0x1010100 && base < 0x1010200)
                {
                    base -= 0x1010100;
                    for (n = 0; n < count; n++)
                    {
                         VSRam[(base + n) ^ 1] = d[n];
                    }
                    return;
                }
            }

            if (base < 6 * 1024 * 1024)
            {
                // memcpy(&Rom_Data[base], data, count);
                for (n = 0; n < count; n++)
                {
                    Rom_Data[(base + n) ^ 1] = d[n];
                }
            }
            else
            {
                for (n = 0; n < count; n++)
                {
                    M68K_WB(base + n, *d++);
                }
            }
        }
        else
        {
            // XXX - Support SegaCD sub CPU write
        }
    }

    void Stop(void)
    {
        Paused = 1;
        {
            Sleep(10);
        }
        m_controller->InjectSignal(2);
    }

    void Step(void)
    {
        unsigned int old_pc = (is_main ? main68k_readPC() : sub68k_readPC());
        unsigned int new_pc;
        unsigned int cycles = 1;
        unsigned int return_code;

        check_breakpoint();

        is_main ? main68k_tripOdometer() : sub68k_tripOdometer();
        do
        {
            return_code = (is_main ? main68k_exec(cycles) : sub68k_exec(cycles));
            new_pc = (is_main ? main68k_readPC() : sub68k_readPC());
            cycles++;
        } while (old_pc == new_pc && return_code >= 0x80000000);

        m_controller->InjectSignal(5);
    }

    void SetPC(unsigned int new_pc)
    {
        S68000CONTEXT &context = is_main ? main68k_context : sub68k_context;

        // XXX - This is probably horribly wrong, but I haven't found another way to do this
        context.pc = new_pc;
    }

    void Continue(void)
    {
        check_breakpoint();
        Paused = 0;
    }

    void Kill(void)
    {
        if (is_main)
        {

        }
    }

    void Disconnect(void)
    {
        if (s_controller != NULL)
        {
            s_controller->Disconnect();
        }
    }

    unsigned int GetMemoryRegions(unsigned int * base, unsigned int * size, unsigned int * flags)
    {
        if (is_main)
        {
            // ROM
            base[0] = 0;
            size[0] = 4096 * 1024;
            flags[0] = 1;

            // RAM
            base[1] = 0xFF0000;
            size[1] = 0x010000;
            flags[1] = 0;

            // VRAM: 64KB at 0x1000000
            base[2] = 0x1000000;
            size[2] = 0x0010000;
            flags[2] = 0;

            // CRAM: 256 bytes at 0x1010000
            base[3] = 0x1010000;
            size[3] = 0x100;
            flags[3] = 0;

            // VSRAM: 256 bytes at 0x1010100
            base[4] = 0x1010100;
            size[4] = 0x100;
            flags[4] = 0;

            return 5;
        }
        else
        {
            return 0;
        }
    }

protected:
    gdbTargetController * m_controller;

    static gdbTargetController * s_controller;

    static bool            s_broke;
    static unsigned int    s_bkpt_pc;

    static void reset_handler(void)
    {
        unsigned int vector = 0;
        unsigned short opcode = 0;
        Paused = 1;
        Sleep(10);

        s_bkpt_pc = (is_main ? main68k_readPC() : sub68k_readPC()) - 2;
        s_broke = true;

        if (is_main)
        {
            main68k_releaseTimeslice();
            main68k_tripOdometer();
            main68k_releaseCycles();
            opcode = main68k_fetch(s_bkpt_pc);
        }
        else
        {
            sub68k_releaseTimeslice();
            sub68k_tripOdometer();
            // There doesn't seem to be a 'sub68k_releaseCycles' - not sure
            // why - haven't investigated.
            // sub68k_releaseCycles();
            opcode = sub68k_fetch(s_bkpt_pc);
        }

        vector = (opcode & 0xF);

        switch (vector)
        {
            case 4:
                s_controller->InjectSignal(vector);
                break;
            case 15:
                s_controller->InjectSignal(5);
                break;
            default:
                s_controller->InjectSignal(143);
                break;
        }
    }

    void check_breakpoint()
    {
        if (s_broke)
        {
            SetPC(s_bkpt_pc);
            s_broke = false;
            s_bkpt_pc = 0;
        }
    }
};

template <const bool is_main>
gdb68KTarget<is_main>::gdb68KTarget(void)
    : m_controller(0)
{

}

template <const bool is_main>
gdbTargetController* gdb68KTarget<is_main>::s_controller;

template <const bool is_main>
bool gdb68KTarget<is_main>::s_broke(false);

template <const bool is_main>
unsigned int gdb68KTarget<is_main>::s_bkpt_pc(0);

template <const bool is_main>
gdb68KTarget<is_main>::~gdb68KTarget(void)
{

}

gdbTarget * GetMain68KTarget()
{
    static gdb68KTarget<true> main68k_target;
    return &main68k_target;
}

gdbTarget * GetSub68KTarget()
{
    static gdb68KTarget<false> sub68k_target;
    return &sub68k_target;
}
