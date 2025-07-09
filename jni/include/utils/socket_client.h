#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <stddef.h> // for size_t

class SocketClient
{
private:
    int sock_fd;

public:
    SocketClient();
    ~SocketClient();

    bool connect_to_server(const char *socket_name);
    bool send_raw(const void *data, size_t size);
    bool receive_raw(void *buffer, size_t size);
    void close_connection();
};

#endif // SOCKET_CLIENT_H
