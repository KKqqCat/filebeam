//
// Created by Singleton on 2026/9/7.
//

#ifndef NETBASE_CORE_HPP
#define NETBASE_CORE_HPP


#include <cstdint>
#include <string>

namespace Netbase {
    namespace core {
        std::string recv_all(int socket_fd, uint64_t len, uint64_t once_recv_size = 256);
        void send_all(int socket_fd, const char* message, uint64_t len, uint64_t once_send_size = 256);
    }
}
#endif //NETBASE_CORE_HPP
