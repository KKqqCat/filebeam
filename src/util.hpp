//
// Created by Singleton on 2026/7/31.
//

#ifndef FILEBEAM_UTIL_HPP
#define FILEBEAM_UTIL_HPP
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <functional>

#define max_read_block_size (1024 * 1024)
#define max_write_block_size (1024 * 1024)
#define max_transfer_size (1ULL << 30)

namespace util {
    std::array<char, 8> uint64_to_bytes(uint64_t value);
    uint64_t bytes_to_uint64(std::array<char, 8> bytes);

    using ProgressFunc = std::function<void(uint64_t done, uint64_t total)>;

    std::string recv_all(int socket_fd, uint64_t len, uint64_t once_recv_size = 256);
    void send_all(int socket_fd, const char* message, uint64_t len, uint64_t once_send_size = 256);


    struct TcpHeader {
        std::string file_name {};
        uint64_t file_size {};
    };

    TcpHeader send_header(int socket_fd, const char* file_path);
    TcpHeader recv_header(int socket_fd);

    void send_file(int socket_fd, const TcpHeader& header, std::ifstream& file, const ProgressFunc& on_progress = nullptr);
    void recv_file(int socket_fd, const TcpHeader& header, const ProgressFunc& on_progress = nullptr);

}


#endif //FILEBEAM_UTIL_HPP
