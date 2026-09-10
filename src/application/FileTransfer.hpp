//
// Created by Singleton on 2026/9/7.
//

#ifndef FILEBEAM_FILETRANSFER_HPP
#define FILEBEAM_FILETRANSFER_HPP

#include <cstdint>
#include <string>
#include <fstream>
#include <functional>

namespace FileTransfer {
    using ProgressFunc = std::function<void(uint64_t done, uint64_t total)>;
    struct TcpHeader {
        std::string file_name {};
        uint64_t file_size {};
    };

    TcpHeader send_header(int socket_fd, const char* file_path);
    TcpHeader recv_header(int socket_fd);

    void send_file(int socket_fd, const TcpHeader& header, std::ifstream& file, const ProgressFunc& on_progress = nullptr);
    void recv_file(int socket_fd, const TcpHeader& header, const ProgressFunc& on_progress = nullptr);

} // FileTransfer

#endif //FILEBEAM_FILETRANSFER_HPP
