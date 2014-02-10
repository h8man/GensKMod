#include "gdb_server_thread.h"

#include "gdb_debug.h"

unsigned int gdbServerThread::Execute(void)
{
    m_socket.Listen(m_port);

    do {
        gdbSocket * sock = m_socket.Accept();
        if (sock != 0)
        {
            debug_printf("Accepted incomming connection\n");
            m_server.Handle(sock, m_target);
            debug_printf("Connection closed\n");
            delete sock;
        }
        else
        {
            debug_printf("No connection... waiting");
            Sleep(1000);
        }
    } while (!m_terminated);

    return 0;
}
