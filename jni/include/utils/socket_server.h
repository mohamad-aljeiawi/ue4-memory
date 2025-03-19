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

class SocketServer {
private:
    const char* socket_path;
    int server_socket;
    int client_socket;
    std::thread accept_thread;
    std::thread receive_thread;
    std::atomic<bool> is_running;
    std::mutex data_mutex;
    std::vector<std::string> command_queue;
    std::function<void(const std::string&)> command_callback;

public:
    SocketServer(const char* path) : socket_path(path), server_socket(-1), client_socket(-1), is_running(false) {}
    
    ~SocketServer() {
        stop();
    }
    
    bool start() {
        // إنشاء سوكت
        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket == -1) {
            LOGE("Failed to create socket");
            return false;
        }
        
        // حذف السوكت إذا كان موجوداً
        unlink(socket_path);
        
        // إعداد عنوان السوكت
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
        
        // ربط السوكت بالعنوان
        if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
            LOGE("Failed to bind socket");
            close(server_socket);
            return false;
        }
        
        // الاستماع للاتصالات
        if (listen(server_socket, 5) == -1) {
            LOGE("Failed to listen on socket");
            close(server_socket);
            return false;
        }
        
        // تعيين الحقوق على ملف السوكت للسماح للتطبيق بالاتصال
        chmod(socket_path, 0666);
        
        is_running = true;
        
        // بدء خيط استقبال الاتصالات
        accept_thread = std::thread(&SocketServer::accept_connections, this);
        
        LOGI("Socket server started at %s", socket_path);
        return true;
    }
    
    void stop() {
        is_running = false;
        
        if (accept_thread.joinable()) {
            accept_thread.join();
        }
        
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        
        if (client_socket != -1) {
            close(client_socket);
            client_socket = -1;
        }
        
        if (server_socket != -1) {
            close(server_socket);
            server_socket = -1;
        }
        
        unlink(socket_path);
        LOGI("Socket server stopped");
    }
    
    bool send_data(const std::string& data) {
        if (client_socket != -1) {
            ssize_t bytes_sent = send(client_socket, data.c_str(), data.length(), 0);
            return (bytes_sent != -1);
        }
        return false;
    }
    
    // إرسال بيانات بتنسيق JSON
    bool send_json(const std::string& json_data) {
        // إضافة طول البيانات في البداية متبوعاً بالفاصلة ثم البيانات
        std::string frame = std::to_string(json_data.length()) + "," + json_data;
        return send_data(frame);
    }
    
    // تعيين دالة استدعاء لمعالجة الأوامر الواردة
    void set_command_callback(std::function<void(const std::string&)> callback) {
        command_callback = callback;
    }
    
    // الحصول على الأمر التالي من الطابور (إذا وجد)
    bool get_next_command(std::string& cmd) {
        std::lock_guard<std::mutex> lock(data_mutex);
        if (command_queue.empty()) {
            return false;
        }
        
        cmd = command_queue.front();
        command_queue.erase(command_queue.begin());
        return true;
    }
    
private:
    void accept_connections() {
        while (is_running) {
            LOGI("Waiting for client connection...");
            client_socket = accept(server_socket, NULL, NULL);
            
            if (client_socket == -1) {
                LOGE("Failed to accept connection");
                continue;
            }
            
            LOGI("Client connected");
            
            // بدء خيط لاستقبال البيانات
            if (receive_thread.joinable()) {
                receive_thread.join();
            }
            receive_thread = std::thread(&SocketServer::receive_data, this);
            
            // إرسال رسالة ترحيب للعميل
            send_data("CONNECTED");
        }
    }
    
    void receive_data() {
        char buffer[4096];
        ssize_t bytes_received;
        
        while (is_running && client_socket != -1) {
            bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_received <= 0) {
                // انقطع الاتصال أو حدث خطأ
                LOGI("Client disconnected");
                close(client_socket);
                client_socket = -1;
                break;
            }
            
            // إضافة الحرف الصفري لنهاية السلسلة
            buffer[bytes_received] = '\0';
            
            std::string command(buffer);
            LOGI("Received command: %s", command.c_str());
            
            // إضافة الأمر إلى الطابور
            {
                std::lock_guard<std::mutex> lock(data_mutex);
                command_queue.push_back(command);
            }
            
            // استدعاء الدالة المرتبطة بالأوامر (إذا تم تعيينها)
            if (command_callback) {
                command_callback(command);
            }
        }
    }
};

#endif // SOCKET_SERVER_H