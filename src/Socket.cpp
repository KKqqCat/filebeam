//
// Created by Singleton on 2026/7/31.
//

#include "Socket.hpp"

#include <charconv>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdexcept>
#include <iostream>


void Socket::Bind(const std::string &addr_v4, const std::string &port) const {
    sockaddr_in sockaddr_in{};
    sockaddr_in.sin_family = AF_INET;
    inet_pton(
        AF_INET,
        addr_v4.data(),
        &sockaddr_in.sin_addr.s_addr
        );
    uint16_t port_int = 0;
    std::from_chars(port.data(), port.data() + port.size(), port_int);
    sockaddr_in.sin_port = htons(port_int);
    int ret = bind(socket_fd_, reinterpret_cast<sockaddr*>(&sockaddr_in), sizeof(sockaddr_in));
    if (ret == -1) {
        throw std::runtime_error("failed to bind socket");
    }
}

void Socket::Listen(int flags) const {
    if (listen(socket_fd_, 10) == -1) {
        throw std::runtime_error("failed to make listen socket");
    }
}

void Socket::Connect(const std::string &addr_v4, const std::string &port) const {
    sockaddr_in sockaddr_in{};
    sockaddr_in.sin_family = AF_INET;
    inet_pton(
        AF_INET,
        addr_v4.data(),
        &sockaddr_in.sin_addr.s_addr);

    uint16_t port_int = 0;
    std::from_chars(port.data(), port.data() + port.size(), port_int);
    sockaddr_in.sin_port = htons(port_int);

    int ret = connect(socket_fd_, reinterpret_cast<sockaddr*>(&sockaddr_in), sizeof(sockaddr_in));
    if (ret == -1) {
        throw std::runtime_error("failed to connect server");
    }
}


Socket Socket::Accept() const {
    int client_fd = accept(socket_fd_, nullptr, nullptr);

    if (client_fd == -1) {
        throw std::runtime_error("failed to build client socket, accept error");
    }
    std::cout << "succeed in connecting client" << std::endl;

    return Socket{client_fd};
}
