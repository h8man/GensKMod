#include "gdb_server.h"
#include "gdb_debug.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif /* _CRT_SECURE_NO_WARNINGS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void bytes_to_hex(const unsigned char * bytes, char * hex, unsigned int length)
{
    char * q = hex;
    static const char hex_digit[] = "0123456789abcdef";
    unsigned int n;

    for (n = 0; n < length; n++)
    {
        *q++ = hex_digit[bytes[n] >> 4];
        *q++ = hex_digit[bytes[n] & 0xF];
    }
    *q = 0;
}

static inline void hex_to_bytes(const char * hex, unsigned char * bytes, unsigned int length)
{
    unsigned char * q = bytes;
    unsigned int n;
    int v;
    char c;

    for (n = 0; n < length; n++)
    {
        c = hex[n * 2];
        v = (c >= '0' && c <= '9') ? (c - '0') :
            (c >= 'a' && c <= 'f') ? (c - 'a' + 10) :
            (c - 'A' + 10);
        v <<= 4;
        c = hex[n * 2 + 1];
        v += (c >= '0' && c <= '9') ? (c - '0') :
             (c >= 'a' && c <= 'f') ? (c - 'a' + 10) :
             (c - 'A' + 10);
        bytes[n] = (unsigned char)v;
    }
}

gdbServerConnection::gdbServerConnection()
    : m_socket(0),
      m_pending_signal(0),
      m_pending_close(false)
{

}

void gdbServerConnection::Handle(gdbSocket * sock, gdbTarget * target)
{
    char data[512];
    int data_received;
    static const char ack[] = "+";
    static const char nack[] = "-";
    static const char unrecognized_response[] = "$#00";
    bool connected = true;

    m_socket = sock;
    m_target = target;
    m_send_buffer = new char [m_target->GetRegisterCount() * 8 + 1];

    m_target->Attach(this);
    m_target->Stop();

    m_socket->SetTimeout(10);
    m_socket->SetBlocking(false);

    m_memory_map_xml = new char[4096];
    unsigned int mem_bases[64];
    unsigned int mem_sizes[64];
    unsigned int mem_flags[64];
    unsigned int mem_regions;
    unsigned int i;

    static const char memory_map_xml_header[] =
        "<?xml version=\"1.0\"?>\n"
        "<!DOCTYPE memory-map\n"
        "          PUBLIC \"+//IDN gnu.org//DTD GDB Memory Map V1.0//EN\"\n"
        "          \"http://sourceware.org/gdb/gdb-memory-map.dtd\">\n"
        "<memory-map>\n";

    memcpy(m_memory_map_xml, memory_map_xml_header, sizeof(memory_map_xml_header));

    mem_regions = m_target->GetMemoryRegions(mem_bases, mem_sizes, mem_flags);

    for (i = 0; i < mem_regions; i++)
    {
        sprintf(data,
                "    <memory type=\"%s\" start=\"0x%08X\" length=\"0x%08X\"/>\n",
                (mem_flags[i] & 1) ? "rom" : "ram",
                mem_bases[i],
                mem_sizes[i]);
        strcat(m_memory_map_xml, data);
    }

    strcat(m_memory_map_xml, "</memory-map>\n");

    // GDB seems to send an ACK as its first character
    m_socket->Receive(data, 1);

    do {
        // If we have been asked to close the connection
        if (m_pending_close)
        {
            // Send the "I am terminated" packet, close the socket
            // and acknowlege. Break out of the loop.
            SendPacketUntilAck("W00");
            m_socket->Close();
            m_pending_close = false;
            break;
        }

        // Clear the data buffer
        memset(data, 0, sizeof(data));

        // Receive a packet
        connected = m_socket->Receive(data, sizeof(data), data_received);
        if (data_received > 0)
        {
            if ((data[0] == '+') && (data_received == 1))
            {
                // Unexpected ACK... just ignore it.
                debug_printf("Unexpected ACK\n");
            } else
            // The plain old character '\003' (0x03) means stop.
            if ((data[0] == 0x03) && (data_received == 1))
            {
                debug_printf("Received stop (0x03)\n");
                SendAck();
                m_target->Stop();
            } else
            // All GDB messages that we know about start with a $ and have at least one more character
            // If we don't get that, there must be a problem... send a nack
            if ((data[0] != '$') || (data_received < 2))
            {
                data[data_received] = 0;
                debug_printf("Unexpected data received: %s (%d bytes)\n", data, data_received);
                // Send nack
                SendNack();
            }
            else
            {
                // Send an ack
                data[511] = 0;
                debug_printf("Received: %s\n", data);
                SendAck();
                switch (data[1])
                {
                    case '!':
                        SendPacketUntilAck("OK");
                        break;
                    case '?':
                        SendPacketUntilAck("S02");
                        break;
                    case 'q':
                        switch (data[2])
                        {
                            case 'A':
                                {
                                    static const char qAttached[] = "$qAttached";
                                    if (memcmp(data, qAttached, sizeof(qAttached) - 1) == 0)
                                    {
                                        SendPacketUntilAck("1");
                                    }
                                    else
                                    {
                                        debug_printf("Query for unknown A feature\n");
                                        SendPacketUntilAck("");
                                    }
                                }
                                break;
                            case 'c':
                                HandleContinue(data);
                                break;
                            case 'C':
                                SendPacketUntilAck("QC0000");
                                break;
                            case 'S':
                                {
                                    static const char qSymbol[] = "$qSymbol";
                                    static const char qSupported[] = "$qSupported";
                                    if (memcmp(data, qSymbol, sizeof(qSymbol) - 1) == 0)
                                    {
                                        SendPacketUntilAck("OK");
                                    }
                                    else if (memcmp(data, qSupported, sizeof(qSupported) - 1) == 0)
                                    {
                                        // Pretend not to support this packet because GDB can't
                                        // handle the wrapping 24-bit address space of the 68K.
                                        // SendPacketUntilAck("qXfer:memory-map:read+");
                                        SendPacketUntilAck("");
                                    }
                                    else
                                    {
                                        debug_printf("Query for unknown S feature\n");
                                        SendPacketUntilAck("");
                                    }
                                }
                                break;
                            case 'X':
                                {
                                    static const char qXfer_mem[] = "$qXfer:memory-map:read";
                                    if (memcmp(data, qXfer_mem, sizeof(qXfer_mem) - 1) == 0)
                                    {
                                        // $qXfer:memory-map:read::0,18a#b4
                                        unsigned int start = 0;
                                        unsigned int length = 0;
                                        sscanf(data, "$qXfer:memory-map:read::%x,%x", &start, &length);
                                        data[0] = (start + length) >= strlen(m_memory_map_xml) ? 'l' : 'm';
                                        memcpy(&data[1], m_memory_map_xml + start, length);
                                        data[length + 2] = 0;
                                        SendPacketUntilAck(data);
                                    }
                                    else
                                    {
                                        debug_printf("Query for unknown X feature\n");
                                    }
                                }
                                break;
                            default:
                                debug_printf("Query for unknown feature\n");
                                SendPacketUntilAck("");
                                break;
                        }
                        break;
                    case 'c':
                        HandleContinue(data);
                        break;
                    case 'D':
                        HandleDetatch(data);
                        break;
                    case 'g':
                        HandleAllRegisterRead(data);
                        break;
                    case 'p':
                        HandleSingleRegisterRead(data);
                        break;
                    case 'P':
                        HandleSingleRegisterWrite(data);
                        break;
                    case 'm':
                        HandleMemoryRead(data);
                        break;
                    case 'M':
                        HandleMemoryWrite(data);
                        break;
                    case 's':
                        HandleStep(data);
                        break;
                    case 'H':
                        SendPacketUntilAck("OK");
                        break;
                    case 'k':
                        // Kill
                        break;
                    default:
                        debug_printf("Unrecognized message\n");
                        SendPacketUntilAck("");
                        break;
                }
            }
        }
        else if (m_pending_signal)
        {
            debug_printf("Signal %d pending\n", m_pending_signal);
            Signal(m_pending_signal);
            m_pending_signal = 0;
            data_received = 1;
        }
        else
        {
            Sleep(2);
        }
    } while (connected);

    delete[] m_memory_map_xml;
    delete[] m_send_buffer;

    printf("Socket no longer connected\n");

    m_target = NULL;
    m_socket = NULL;
}

void gdbServerConnection::Signal(unsigned int signum)
{
    char buffer[8];
    sprintf(buffer, "S%02X", signum & 0xFF);
    SendPacketUntilAck(buffer);
}

void gdbServerConnection::Disconnect(void)
{
    m_pending_close = true;
}

// Handle a single register read. Format '$pNN#cc', NN = reg number, cc == checksum
void gdbServerConnection::HandleSingleRegisterRead(const char * packet)
{
    char reg_str[] = { '0', 'x', packet[2], packet[3], 0 };
    int regnum;
    unsigned int regval;
    char buffer[9];

    if (reg_str[3] == '#')
        reg_str[3] = 0;
    regnum = strtol(reg_str, NULL, 16);

    regval = m_target->GetRegisterValue(regnum);

    sprintf(buffer, "%08X", regval);

    SendPacketUntilAck(buffer);
}

// Handle a single register write. Format $pNN=MM#CC. NN = reg, MM = val, CC = checksum
void gdbServerConnection::HandleSingleRegisterWrite(const char * packet)
{
    char reg_str[12];
    char val_str[12];
    unsigned int reg;
    unsigned int val;
    const char * p;
    char * q;

    q = reg_str;
    p = packet + 2;

    while (*p != '=')
    {
        *q++ = *p++;
    }
    *q = 0;
    p++;

    q = val_str;

    while (*p != '#')
    {
        *q++ = *p++;
    }
    *q = 0;

    reg = strtoul(reg_str, NULL, 16);
    val = strtoul(val_str, NULL, 16);

    m_target->SetRegisterValue(reg, val);

    SendPacketUntilAck("OK");
}

// Handle all register read.
void gdbServerConnection::HandleAllRegisterRead(const char * packet)
{
    char * p = m_send_buffer;

    int n, m;

    *p = 0;
    m = m_target->GetRegisterCount();

    for (n = 0; n < m; n++)
    {
        sprintf(p, "%08X", m_target->GetRegisterValue(n));
        p += 8;
    }

    SendPacketUntilAck(m_send_buffer);
}

// Memory read: $mNNNN,MMMMM#CC
void gdbServerConnection::HandleMemoryRead(const char * packet)
{
    char base_str[12];
    char length_str[12];
    unsigned char buffer[1024];
    unsigned int base;
    unsigned int length;

    const char * p;
    char * q;

    q = base_str;
    p = packet + 2;
    while (*p != ',')
    {
        *q++ = *p++;
    }
    *q = 0;
    p++;

    q = length_str;
    while (*p != '#')
    {
        *q++ = *p++;
    }
    *q = 0;

    char * send_buffer = m_send_buffer;
    base = strtol(base_str, NULL, 16);
    length = strtol(length_str, NULL, 16);

    static const char hex[] = "0123456789abcdef";

    if (length > 32)
    {
        send_buffer = new char [length * 2 + 1];
    }

    while (length != 0)
    {
        unsigned int bytes = length > 256 ? 256 : length;
        debug_printf("Reading %d bytes from memory at address 0x%08X\n", bytes, base);
        m_target->ReadMemory(base, bytes, buffer);
        bytes_to_hex(buffer, send_buffer, bytes);
        base += bytes;
        length -= bytes;
    }

    *q = 0;
    SendPacketUntilAck(send_buffer);

    if (send_buffer != m_send_buffer)
        delete [] send_buffer;
}

// Memory read: $MNNNN,MMMMM:VVVV...#CC - NNNNN = base, MMMM = count, VVVV - data
void gdbServerConnection::HandleMemoryWrite(const char * packet)
{
    char base_str[12];
    char length_str[12];
    unsigned char buffer[1024];
    unsigned int base, length;

    const char * p;
    char * q;

    q = base_str;
    p = packet + 2;
    while (*p != ',')
    {
        *q++ = *p++;
    }
    *q = 0;
    p++;

    q = length_str;
    while (*p != ':')
    {
        *q++ = *p++;
    }
    *q = 0;
    p++;

    base = strtol(base_str, NULL, 16);
    length = strtol(length_str, NULL, 16);

    while (length != 0)
    {
        unsigned int bytes = length > 1024 ? 1024 : length;
        hex_to_bytes(p, buffer, bytes);
        p += bytes * 2;
        debug_printf("Writing %d bytes to memory at address 0x%08X\n", bytes, base);
        m_target->WriteMemory(base, bytes, buffer);
        base += bytes;
        length -= bytes;
    }

    SendPacketUntilAck("OK");
}

// Single step. Might be '$s#CC' or $sNNNN#CC' - NNNNNN is continuation address
void gdbServerConnection::HandleStep(const char * packet)
{
    debug_printf("Got step %s\n", packet);

    if (packet[2] != '#')
    {
        // Address specified - XXX - extract address from step packet
        m_target->SetPC(0);
    }

    m_target->Step();
}

void gdbServerConnection::HandleContinue(const char * packet)
{
    m_target->Continue();

    // NO reply to continue - target will stop and call ::Signal at the right time.
}

void gdbServerConnection::HandleDetatch(const char * packet)
{
    m_target->Continue();

    SendPacketUntilAck("OK");
}

void gdbServerConnection::HandleKill(const char * packet)
{
    m_target->Kill();
}

void gdbServerConnection::SendPacket(const char * str)
{
    int length = strlen(str);
    char buffer[256];
    char * p = buffer;
    static const char hex[] = "0123456789ABCDEF";

    // Socket might get disconnected - return early
    if (!m_socket)
        return;

    // Format of buffer is $ <str> # <chksum>. So, we need 4 chars more than strlen(str)
    if (length > sizeof(buffer) - 4)
    {
        p = new char [length + 4];
    }

    // Assemble packet
    p[0] = '$';
    memcpy(p + 1, str, length);
    p[length + 1] = '#';

    // Checksum
    int checksum = 0;
    int n;

    for (n = 0; n < length; n++)
    {
        checksum += (int)str[n];
    }

    // Append checksum
    p[length + 2] = hex[(checksum >> 4) & 0xF];
    p[length + 3] = hex[checksum & 0xF];

    // Send
    m_socket->Send(p, length + 4);
    if (length > 16)
    {
        p[16] = '.';
        p[17] = '.';
        p[18] = '.';
        p[19] = 0;
    }
    else
    {
        p[length + 3] = 0;
    }
    debug_printf("Sent packet %s\n", p);

    // Delete if we couldn't use local packet buffer
    if (p != buffer)
        delete [] p;
}

void gdbServerConnection::InjectSignal(unsigned int signum)
{
    m_pending_signal = signum;
}
