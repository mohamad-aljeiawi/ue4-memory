#include "utils/socket_server.h"
#include "debug/logger.h"

#include <cerrno>
#include <cstring>
#include <utility>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

static void make_socket_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

SocketServer::SocketServer(std::string name) : socket_name(std::move(name)), server_socket(-1),
                                               client_socket(-1) {}

SocketServer::~SocketServer()
{
    stop();
}

bool SocketServer::start()
{
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        Logger::e("[Server] Failed to create socket: %s", strerror(errno));
        return false;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    addr.sun_path[0] = '\0'; // abstract socket
    memcpy(addr.sun_path + 1, socket_name.c_str(), socket_name.size());
    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + 1 + socket_name.size();

    if (bind(server_socket, (struct sockaddr *)&addr, addr_len) == -1)
    {
        Logger::e("[Server] Failed to bind: %s", strerror(errno));
        close(server_socket);
        return false;
    }

    make_socket_non_blocking(server_socket);

    if (listen(server_socket, 1) == -1)
    {
        Logger::e("[Server] Failed to listen: %s", strerror(errno));
        close(server_socket);
        return false;
    }

    Logger::i("[Server] SocketServer started successfully.");
    return true;
}

void SocketServer::stop()
{
    if (client_socket != -1)
    {
        close(client_socket);
        client_socket = -1;
    }

    if (server_socket != -1)
    {
        close(server_socket);
        server_socket = -1;
    }

    Logger::i("[Server] SocketServer stopped.");
}

bool SocketServer::send_raw(const void *data, size_t size)
{
    if (client_socket == -1)
        return false;

    const char *ptr = static_cast<const char *>(data);
    size_t sent = 0;
    while (sent < size)
    {
        ssize_t n = send(client_socket, ptr + sent, size - sent, 0);
        if (n <= 0)
            return false;
        sent += n;
    }
    return true;
}

bool SocketServer::receive_raw(void *buffer, size_t size)
{
    if (client_socket == -1)
        return false;

    char *ptr = static_cast<char *>(buffer);
    size_t received = 0;
    while (received < size)
    {
        ssize_t n = recv(client_socket, ptr + received, size - received, 0);
        if (n <= 0)
            return false;
        received += n;
    }
    return true;
}