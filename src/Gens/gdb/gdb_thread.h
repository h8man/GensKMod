#ifndef __GDB_THREAD_H__
#define __GDB_THREAD_H__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#define WIN32_EXTRA_LEAN 1
#include <windows.h>
#endif /* WIN32 */

class gdbThread
{
public:
    gdbThread()
        : m_thread(0),
          m_terminated(false)
    {

    }

    virtual ~gdbThread()
    {

    }

    void Run(void);
    virtual void Terminate(void);
    void Wait(void);

    virtual unsigned int Execute(void) = 0;

protected:
#ifdef WIN32
    HANDLE      m_thread;
    DWORD       m_thread_id;
    static DWORD WINAPI ThreadEntry(LPVOID self);
#endif
    bool        m_terminated;
};

#endif /* __GDB_THREAD_H__ */
