#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <string>
#include <atomic>
#include <thread>

class SocketServer
{
public:
    explicit SocketServer(std::string name);
    ~SocketServer();

    bool start();
    void stop();
    bool send_raw(const void *data, size_t size);
    bool receive_raw(void *buffer, size_t size);

    int server_socket;
    int client_socket;

private:
    std::string socket_name;
};

#endif // SOCKET_SERVER_H
