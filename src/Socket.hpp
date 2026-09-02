//
// Created by Singleton on 2026/7/31.
//

#ifndef FILEBEAM_SOCKET_HPP
#define FILEBEAM_SOCKET_HPP
#include <sys/socket.h>
#include <string>
#include <unistd.h>

class Socket {
public:
    Socket() {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    }

    ~Socket() {
        if (is_valid_) {
            close(socket_fd_);
        }
    }

    Socket(Socket&& other_socket)  noexcept {
        // 转移所有权
        socket_fd_ = other_socket.socket_fd_;
        other_socket.is_valid_ = false;
        is_valid_ = true;
    }
    Socket& operator=(Socket&& other_socket)  noexcept {
        // 关闭原socket
        if (is_valid_) {
            close(socket_fd_);
        }
        socket_fd_ = other_socket.socket_fd_;
        other_socket.is_valid_ = false;
        is_valid_ = true;
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
    int socket_fd_ = 0;
    bool is_valid_ = true;

    explicit Socket(int socket_fd) {
        socket_fd_ = socket_fd;
    }
};


#endif //FILEBEAM_SOCKET_HPP
