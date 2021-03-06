#ifndef __GDB_TARGET_H__
#define __GDB_TARGET_H__

class gdbTargetController
{
public:
    virtual void Signal(unsigned int signal) = 0;
    virtual void InjectSignal(unsigned int signal) = 0;
    virtual void Disconnect(void) = 0;
};

class gdbTarget
{
public:
    virtual void Attach(gdbTargetController * controller) { }
    virtual int GetRegisterCount() { return 0; }
    virtual unsigned int GetRegisterValue(int index) { return 0; }
    virtual void SetRegisterValue(int index, unsigned int value) { }
    virtual void ReadMemory(unsigned int base, unsigned int count, void * data) { }
    virtual void WriteMemory(unsigned int base, unsigned int count, const void * data) { }
    virtual unsigned int GetMemoryRegions(unsigned int * base, unsigned int * size, unsigned int * flags) { return 0; }
    virtual void Stop(void) { }
    virtual void Step(void) { }
    virtual void SetPC(unsigned int new_pc) { }
    virtual void Continue(void) { }
    virtual void Kill(void) { }
    virtual void Disconnect(void) { }
};

#endif /* __GDB_TARGET_H__ */
