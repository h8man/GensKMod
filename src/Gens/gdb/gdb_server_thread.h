#ifndef __GDB_SERVER_THREAD_H__
#define __GDB_SERVER_THREAD_H__

#include "gdb_thread.h"
#include "gdb_server.h"

class gdbServerThread : public gdbThread
{
public:
    gdbServerThread(unsigned short port, gdbTarget * target)
        : m_port(port),
          m_target(target)
    {
    }
    ~gdbServerThread() { }

    unsigned int Execute(void);

protected:
    gdbSocket           m_socket;
    gdbServerConnection m_server;
    gdbTarget           * m_target;
    unsigned short      m_port;
};

#endif /* __GDB_SERVER_THREAD_H__ */
