#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include "logger.h"

class SocketServer
{
private:
    const char *socket_path;
    int server_socket;
    int client_socket;
    std::thread accept_thread;
    std::thread receive_thread;
    std::atomic<bool> is_running;
    std::mutex data_mutex;
    std::vector<std::string> command_queue;
    std::function<void(const std::string &)> command_callback;

public:
    SocketServer(const char *path) : socket_path(path), server_socket(-1), client_socket(-1), is_running(false) {}

    ~SocketServer()
    {
        stop();
    }

    bool start()
    {
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket == -1)
        {
            return false;
        }

        unlink(socket_path);

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

        if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            close(server_socket);
            return false;
        }

        if (listen(server_socket, 5) == -1)
        {
            close(server_socket);
            return false;
        }

        chmod(socket_path, 0666);

        is_running = true;

        accept_thread = std::thread(&SocketServer::accept_connections, this);
        return true;
    }

    void stop()
    {
        is_running = false;

        if (accept_thread.joinable())
        {
            accept_thread.join();
        }

        if (receive_thread.joinable())
        {
            receive_thread.join();
        }

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

        unlink(socket_path);
    }

    bool send_data(const std::string &data)
    {
        if (client_socket != -1)
        {
            ssize_t bytes_sent = send(client_socket, data.c_str(), data.length(), 0);
            return (bytes_sent != -1);
        }
        return false;
    }

    bool send_json(const std::string &json_data)
    {

        std::string frame = std::to_string(json_data.length()) + "," + json_data;
        return send_data(frame);
    }

    void set_command_callback(std::function<void(const std::string &)> callback)
    {
        command_callback = callback;
    }

    bool get_next_command(std::string &cmd)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        if (command_queue.empty())
        {
            return false;
        }

        cmd = command_queue.front();
        command_queue.erase(command_queue.begin());
        return true;
    }

private:
    void accept_connections()
    {
        while (is_running)
        {
            client_socket = accept(server_socket, NULL, NULL);

            if (client_socket == -1)
            {
                continue;
            }

            if (receive_thread.joinable())
            {
                receive_thread.join();
            }
            receive_thread = std::thread(&SocketServer::receive_data, this);

            send_data("CONNECTED");
        }
    }

    void receive_data()
    {
        char buffer[4096];
        ssize_t bytes_received;

        while (is_running && client_socket != -1)
        {
            bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

            if (bytes_received <= 0)
            {
                close(client_socket);
                client_socket = -1;
                break;
            }

            buffer[bytes_received] = '\0';

            std::string command(buffer);
            {
                std::lock_guard<std::mutex> lock(data_mutex);
                command_queue.push_back(command);
            }

            if (command_callback)
            {
                command_callback(command);
            }
        }
    }
};

#endif // SOCKET_SERVER_H