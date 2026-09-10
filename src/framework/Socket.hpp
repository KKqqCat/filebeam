//
// Created by Singleton on 2026/7/31.
//

#ifndef NETBASE_SOCKET_HPP
#define NETBASE_SOCKET_HPP
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <stdexcept>

namespace Netbase {
    class Socket {
    public:
        Socket() {
            socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
            if (socket_fd_ == -1) {
                throw std::runtime_error("failed to build socket");
            }
        }

        ~Socket() {
            if (socket_fd_ != -1) {
                close(socket_fd_);
            }
        }

        Socket(Socket&& other_socket)  noexcept {
            // 转移所有权
            socket_fd_ = other_socket.socket_fd_;
            other_socket.socket_fd_ = -1;
        }
        Socket& operator=(Socket&& other_socket)  noexcept {
            // 避免自移动赋值
            if (&other_socket == this)
                return *this;
            // 关闭this原socket
            if (socket_fd_ != -1) {
                close(socket_fd_);
            }
            socket_fd_ = other_socket.socket_fd_;
            other_socket.socket_fd_ = -1;
            return *this;
        }

        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        void Bind(const std::string& addr_v4, const std::string& port) const;
        void Listen(int flags = 10) const;
        void Connect(const std::string& addr_v4, const std::string& port) const;

        void SetTimeout(int seconds) const;

        [[nodiscard]] Socket Accept() const;


        [[nodiscard]] int fd() const { return socket_fd_; }

    private:
        int socket_fd_ = -1;

        explicit Socket(int socket_fd) {
            socket_fd_ = socket_fd;
        }
    };
}


#endif //NETBASE_SOCKET_HPP
