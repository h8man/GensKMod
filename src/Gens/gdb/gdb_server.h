#ifndef __GDB_SERVER_H__
#define __GDB_SERVER_H__

#include "gdb_target.h"
#include "gdb_socket.h"
#include "gdb_debug.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif /* _WIN32 */

/**

    gdbServerConnection

        Handles connections from gdb. Parse protocol. Interact with debuggee. Return information to gdb.

**/

class gdbServerConnection : public gdbTargetController
{
public:
    gdbServerConnection();

    void Handle(gdbSocket * sock, gdbTarget * target);
    void Disconnect(void);

public:
    // Functions called by target
    void Signal(unsigned int signum);
    void InjectSignal(unsigned int signum);

protected:
    gdbSocket * m_socket;
    gdbTarget * m_target;
    char * m_send_buffer;
    unsigned int m_pending_signal;
    bool   m_pending_close;
    char * m_memory_map_xml;

    inline bool GotAck(void)
    {
        char ack = 0;
        int bytes;
        bool success = m_socket->Receive(&ack, 1, bytes);

        if (success)
        {
            if (bytes == 1 && ack == '+')
            {
                debug_printf("Got ACK (+)\n");
            }
            else if (bytes > 0)
            {
                debug_printf("Got something other than ACK (0x%02X ('%c')). That was... unexpected.\n", ack, ack ? ack : '0');
            }
            return ack == '+';
        }
        else
        {
            return false;
        }
    }

    inline void SendAck(void)
    {
        static const char ack = '+';

        debug_printf("Sending ACK (+)\n");
        m_socket->Send(&ack, 1);
    }

    inline void SendNack(void)
    {
        static const char nack = '-';

        debug_printf("Sending NACK (-)\n");
        m_socket->Send(&nack, 1);
    }

    void SendPacket(const char * str);
    bool SendPacketUntilAck(const char * str, int max_attempts = 20)
    {
        debug_printf("Sending %s until ACK\n", str);
        int attempts = 0;
        bool ack;
        SendPacket(str);
        do {
            attempts++;
            ack = GotAck();
            if (!ack)
            {
                ::Sleep(20);
            }
        } while (!ack && attempts < max_attempts);

        if (attempts == max_attempts)
        {
            debug_printf("Failed to get ACK\n");
            return false;
        }
        else
        {
            debug_printf("Success after %d attempts\n", attempts);
            return true;
        }
    }

    void HandleSingleRegisterRead(const char * packet);
    void HandleSingleRegisterWrite(const char * packet);
    void HandleAllRegisterRead(const char * packet);
    void HandleMemoryRead(const char * packet);
    void HandleMemoryWrite(const char * packet);
    void HandleStep(const char * packet);
    void HandleContinue(const char * packet);
    void HandleDetatch(const char * packet);
    void HandleKill(const char * packet);
};

#endif /* __GDB_SERVER_H__ */
