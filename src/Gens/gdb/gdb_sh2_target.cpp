#include "gdb_sh2_target.h"

extern "C"
{
#include "../SH2.h"
}
#include "../Mem_SH2.h"

#include "../vdp_io.h"

#include <string.h>

extern "C"
{
extern int Paused;
};

template <const bool is_master>
class gdbSH2Target : public gdbTarget
{
public:
    gdbSH2Target(void)
        : m_controller(0)
    {}
    ~gdbSH2Target(void);

protected:
    gdbTargetController * m_controller;

    void Attach(gdbTargetController * controller)
    {
        m_controller = controller;
    }

    int GetRegisterCount()
    {
        return 21;
    }

    unsigned int GetRegisterValue(int index)
    {
        SH2_CONTEXT* ctx = &(is_master ? M_SH2 : S_SH2);
        if (index >= 0 && index < 15)
        {
            return SH2_Get_R(ctx, index + 1);
        }
        else switch (index)
        {
            case 15: return SH2_Get_SR(ctx);
            case 16: return SH2_Get_PC(ctx);
            case 17: return SH2_Get_PR(ctx);
            case 18: return SH2_Get_GBR(ctx);
            case 19: return SH2_Get_MACH(ctx);
            case 20: return SH2_Get_MACL(ctx);
        }

        return 0;
    }

    void SetRegisterValue(int index, unsigned int value)
    {
        SH2_CONTEXT* ctx = &(is_master ? M_SH2 : S_SH2);
        if (index >= 0 && index < 15)
        {
            ctx->R[index] = value;
        }
        else switch (index)
        {
            case 15: SH2_Set_SR(ctx, value);
            case 16: SH2_Set_PC(ctx, value);
            case 17: SH2_Set_PR(ctx, value);
            case 18: SH2_Set_GBR(ctx, value);
            case 19: SH2_Set_MACH(ctx, value);
            case 20: SH2_Set_MACL(ctx, value);
        }
    }

    void ReadMemory(unsigned int base, unsigned int count, void * data)
    {
        SH2_CONTEXT* ctx = &(is_master ? M_SH2 : S_SH2);
        unsigned char * ptr = (unsigned char *)data;
        unsigned int n;

        for (n = 0; n < count; n++)
        {
            ptr[n] = SH2_Read_Byte(ctx, base + n);
        }
    }

    void Step(void)
    {
        SH2_CONTEXT* ctx = &(is_master ? M_SH2 : S_SH2);

        unsigned int old_pc = SH2_Get_PC(ctx);
        SH2_Clear_Odo(ctx);

        do {
            SH2_Exec(ctx, 1);
        } while (SH2_Get_PC(ctx) == old_pc);

        m_controller->InjectSignal(5);
    }

    void Stop(void)
    {
        Paused = 1;
        m_controller->InjectSignal(2);
    }

    void Continue(void)
    {
        Paused = 0;
    }
};

gdbTarget * GetMasterSH2Target()
{
    return new gdbSH2Target<true>;
}

gdbTarget * GetSlaveSH2Target()
{
    return new gdbSH2Target<false>;
}
