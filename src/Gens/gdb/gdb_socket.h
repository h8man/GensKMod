#ifndef __GDB_SOCKET_H__
#define __GDB_SOCKET_H__

class gdbSocket
{
public:
    gdbSocket(int s = 0);
    virtual ~gdbSocket();

    void Listen(unsigned short port);
    gdbSocket * Accept();
    void Connect();
    void Send(const void * data, int length, int& length_sent);
    void Send(const void * data, int length) { int dummy; Send(data, length, dummy); }
    void SetTimeout(unsigned int ms);
    bool DataAvailable(void);
    bool Receive(void * data, int length, int& length_received);
    int  Receive(void * data, int length) { int dummy; Receive(data, length, dummy); return dummy; }
    void SetBlocking(bool blocking);
    void Close();
    bool IsConnected();

protected:
    static unsigned int s_init_count;

    int m_socket;

    void Initialize();
    void Uninitialize();

private:
};

#endif /* __GDB_SOCKET_H__ */
