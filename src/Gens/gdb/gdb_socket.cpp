#ifdef WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <winsock.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#endif

#include <stdio.h>

#include "gdb_target.h"
#include "gdb_socket.h"
#include "gdb_server.h"

unsigned int gdbSocket::s_init_count = 0;

gdbSocket::gdbSocket(int s)
    : m_socket(s)
{
    Initialize();
}

gdbSocket::~gdbSocket()
{
    Uninitialize();
}

void gdbSocket::Initialize()
{
#ifdef WIN32
    if (InterlockedIncrement((LONG*)&s_init_count) == 1)
    {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }
#endif
}

void gdbSocket::Uninitialize()
{
#ifdef WIN32
    if (InterlockedDecrement((LONG *)&s_init_count) == 0)
    {
        WSACleanup();
    }
#endif
}

void gdbSocket::Listen(unsigned short port)
{
    struct sockaddr_in server_addr;

    m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    bind(m_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(m_socket, 1);
}

gdbSocket * gdbSocket::Accept()
{
    struct sockaddr_in client_addr;
    int client_addr_size = (int)sizeof(client_addr);
    int client_socket;

    client_socket = accept(m_socket, (struct sockaddr *)&client_addr, &client_addr_size);

    if (client_socket != -1)
    {
        return new gdbSocket(client_socket);
    }

    return NULL;
}

void gdbSocket::Close()
{
    closesocket(m_socket);

    m_socket = 0;
}

void gdbSocket::Send(const void * data, int length, int& length_sent)
{
    length_sent = send(m_socket, (const char *)data, (int)length, 0);
}

void gdbSocket::SetTimeout(unsigned int ms)
{
    struct timeval tv;

    tv.tv_sec = ms / 1000000;  /* 30 Secs Timeout */
    tv.tv_usec = ms / 1000;  // Not init'ing this can cause strange errors

    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
}

void gdbSocket::SetBlocking(bool blocking)
{
    u_long val = blocking ? 0 : 1;

    ioctlsocket(m_socket, FIONBIO, &val);
}

bool gdbSocket::DataAvailable(void)
{
    fd_set s;
    FD_ZERO(&s);
    FD_SET(m_socket, &s);

    int n = select(m_socket + 1, &s, NULL, NULL, NULL);

    return (n != -1 && FD_ISSET(m_socket, &s));
}

bool gdbSocket::Receive(void * data, int length, int& length_received)
{
    length_received = recv(m_socket, (char *)data, length, 0);

    if (length_received > 0)
    {
        return true;
    }

    if (length_received < 0)
    {
        int val = WSAGetLastError();

        return (val == WSAEWOULDBLOCK);
    }

    return false;
}

bool gdbSocket::IsConnected()
{
    fd_set read_fs[] = { m_socket };
    TIMEVAL tv = { 0, 0 };

    return select(0, read_fs, NULL, NULL, &tv) >= 0;
}
