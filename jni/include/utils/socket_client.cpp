#include "utils/socket_client.h"
#include "debug/logger.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

SocketClient::SocketClient() : sock_fd(-1) {}

SocketClient::~SocketClient()
{
    close_connection();
}

bool SocketClient::connect_to_server(const char *socket_name)
{
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        Logger::e("socket() failed: %s", strerror(errno));
        return false;
    }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;

    addr.sun_path[0] = '\0'; // Abstract socket prefix
    size_t name_len = strlen(socket_name);
    memcpy(addr.sun_path + 1, socket_name, name_len);

    socklen_t addr_len = offsetof(struct sockaddr_un, sun_path) + 1 + name_len;

    int result = connect(sock_fd, (struct sockaddr *)&addr, addr_len);
    if (result == -1)
    {
        Logger::e("connect() failed: %s", strerror(errno));
        close(sock_fd);
        sock_fd = -1;
        return false;
    }

    return true;
}

bool SocketClient::send_raw(const void *data, size_t size)
{
    const char *ptr = static_cast<const char *>(data);
    size_t sent = 0;

    while (sent < size)
    {
        ssize_t n = send(sock_fd, ptr + sent, size - sent, 0);
        if (n <= 0)
        {
            Logger::e("send() failed at %zu/%zu bytes: %s", sent, size, strerror(errno));
            return false;
        }

        sent += n;
    }

    return true;
}

bool SocketClient::receive_raw(void *buffer, size_t size)
{
    char *ptr = static_cast<char *>(buffer);
    size_t received = 0;
    while (received < size)
    {
        ssize_t n = recv(sock_fd, ptr + received, size - received, 0);
        if (n <= 0)
            return false;
        received += n;
    }
    return true;
}

void SocketClient::close_connection()
{
    if (sock_fd != -1)
    {
        close(sock_fd);
        sock_fd = -1;
    }
}
