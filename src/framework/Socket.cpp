//
// Created by Singleton on 2026/7/31.
//

#include "Socket.hpp"

#include <charconv>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace Netbase {
    // 0 意味着由内核分配
    constexpr uint16_t port_min = 0;
    constexpr uint16_t port_max = 65535;


    void Socket::Bind(const std::string &addr_v4, const std::string &port) const {
        if (socket_fd_ == -1) {
            throw std::runtime_error("use invalid socket");
        }
        sockaddr_in sockaddr_in{};
        sockaddr_in.sin_family = AF_INET;
        if (inet_pton(AF_INET, addr_v4.data(), &sockaddr_in.sin_addr.s_addr) != 1) {
            throw std::runtime_error("Invalid IPv4 address : '" + addr_v4 + "'");
        }
        uint16_t port_int = 0;
        const auto [ptr, ec] = std::from_chars(port.data(), port.data() + port.size(), port_int);
        if (ec != std::errc{} || ptr != port.data() + port.size()) {
            throw std::runtime_error("Invalid port : '" + port + "'");
        }
        if (port_int < port_min || port_int > port_max) {
            throw std::runtime_error("port out of range. size range : " + std::to_string(port_min) + "-" + std::to_string(port_max));
        }

        sockaddr_in.sin_port = htons(port_int);
        int ret = bind(socket_fd_, reinterpret_cast<sockaddr*>(&sockaddr_in), sizeof(sockaddr_in));
        if (ret == -1) {
            throw std::runtime_error("failed to bind socket");
        }
    }

    void Socket::Listen(int flags) const {
        if (socket_fd_ == -1) {
            throw std::runtime_error("use invalid socket");
        }
        if (listen(socket_fd_, flags) == -1) {
            throw std::runtime_error("failed to make listen socket");
        }
    }

    void Socket::Connect(const std::string &addr_v4, const std::string &port) const {
        if (socket_fd_ == -1) {
            throw std::runtime_error("use invalid socket");
        }
        sockaddr_in sockaddr_in{};
        sockaddr_in.sin_family = AF_INET;
        if (inet_pton(AF_INET, addr_v4.data(), &sockaddr_in.sin_addr.s_addr) != 1) {
            throw std::runtime_error("Invalid IPv4 address : '" + addr_v4 + "'");
        }
        uint16_t port_int = 0;
        const auto [ptr, ec] = std::from_chars(port.data(), port.data() + port.size(), port_int);
        if (ec != std::errc{} || ptr != port.data() + port.size()) {
            throw std::runtime_error("Invalid port : '" + port + "'");
        }
        if (port_int < port_min || port_int > port_max) {
            throw std::runtime_error("port out of range. size range : " + std::to_string(port_min) + "-" + std::to_string(port_max));
        }
        sockaddr_in.sin_port = htons(port_int);

        int ret = connect(socket_fd_, reinterpret_cast<sockaddr*>(&sockaddr_in), sizeof(sockaddr_in));
        if (ret == -1) {
            throw std::runtime_error("failed to connect server");
        }
    }

    void Socket::SetTimeout(int seconds) const {
        if (socket_fd_ == -1) {
            throw std::runtime_error("use invalid socket");
        }
        timeval tv{};
        tv.tv_sec = seconds;
        tv.tv_usec = 0;
        if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
            throw std::runtime_error("failed to set SO_RCVTIMEO");
        }
        if (setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
            throw std::runtime_error("failed to set SO_SNDTIMEO");
        }
    }


    Socket Socket::Accept() const{
        if (socket_fd_ == -1) {
            throw std::runtime_error("use invalid socket");
        }
        int client_fd = accept(socket_fd_, nullptr, nullptr);

        if (client_fd == -1) {
            throw std::runtime_error("failed to build client socket, accept error");
        }

        return Socket{client_fd};
    }
}