//
// Created by Singleton on 2026/9/7.
//

#include "core.hpp"
#include <cstdint>
#include <string>
#include <sys/socket.h>
#include <stdexcept>
#include <cerrno>
#include <cstring>

namespace Netbase {
    namespace core {
        constexpr uint64_t max_transfer_size = 1ULL << 30;

        std::string recv_all(int socket_fd, uint64_t len, uint64_t once_recv_size) {
            if (len > max_transfer_size) {
                throw std::runtime_error("once recv size out of range, max size is 1G");
            }

            if (once_recv_size > len) {
                once_recv_size = len;
            }

            uint64_t remaining_size = len;
            std::string ret;
            ret.reserve(len);

            while (remaining_size > 0) {
                uint64_t block_size = remaining_size < once_recv_size ? remaining_size : once_recv_size;
                char buf[block_size];
                ssize_t recv_size = recv(socket_fd, buf, block_size, 0);
                if (recv_size > 0) {
                    ret.append(buf, recv_size);
                    remaining_size -= recv_size;
                }else if (recv_size == 0) {
                    throw std::runtime_error("connection closed when transferring, recv stop");
                }else if (errno == EINTR) {
                    continue;
                }else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    throw std::runtime_error("recv timeout");
                }else {
                    throw std::runtime_error(std::string("failed to recv data, err info : ") + std::strerror(errno));
                }
            }
            return ret;
        }
        void send_all(int socket_fd, const char* message, uint64_t len, uint64_t once_send_size) {
            if (once_send_size > len) {
                once_send_size = len;
            }
            uint64_t remaining_size = len;
            uint64_t sent_size = 0;
            while (sent_size < len) {
                const char *buf = message + sent_size;
                // 一次期望发once_send_size字节，如果剩余字节不足once_send_size，则期望发剩余字节数大小。
                uint64_t block_size = remaining_size < once_send_size ? remaining_size : once_send_size;
                ssize_t send_size = send(socket_fd, buf, block_size, 0);
                if (send_size > 0) {
                    // 更新实际已发送字节数和剩余字节数
                    sent_size += send_size;
                    remaining_size = len - sent_size;
                }else if (send_size == 0) {
                    throw std::runtime_error("connection closed when transferring, send stop");
                }else if (errno == EINTR) {
                    continue;
                }else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    throw std::runtime_error("send timeout");
                }else {
                    throw std::runtime_error(std::string("failed to send data, err info : ") + std::strerror(errno));
                }
            }
        }
    }
}