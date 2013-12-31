#include "gdb_thread.h"

void gdbThread::Run(void)
{
    m_thread = CreateThread(NULL, 0, &ThreadEntry, this, 0, &m_thread_id);
}

void gdbThread::Wait(void)
{
    WaitForSingleObject(m_thread, INFINITE);
}

void gdbThread::Terminate(void)
{
    m_terminated = true;
    if (WaitForSingleObject(m_thread, 1000) == WAIT_TIMEOUT)
        TerminateThread(m_thread, 0);
}

DWORD WINAPI gdbThread::ThreadEntry(LPVOID self)
{
    return reinterpret_cast<gdbThread*>(self)->Execute();
}
