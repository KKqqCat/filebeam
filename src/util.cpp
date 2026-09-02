//
// Created by Singleton on 2026/9/1.
//
#include "util.hpp"

#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <cerrno>
#include <cstring>

namespace util {
    std::array<char, 8> uint64_to_bytes(uint64_t value) {
        std::array<char, 8> bytes{};
        for (int i = 0; i < 8; i++) {
            bytes[i] = static_cast<char>((value >> ((7-i)*8)) & 0xff);
        }
        return bytes;
    }
    uint64_t bytes_to_uint64(std::array<char, 8> bytes) {
        uint64_t value{};
        for (char byte : bytes) {
            value = (value << 8) | static_cast<unsigned char>(byte);
        }
        return value;
    }

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
                throw std::runtime_error("recv timeout : 30s");
            }else {
                throw std::runtime_error(std::string("failed to recv data") + std::strerror(errno));
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
                throw std::runtime_error("send timeout : 30s");
            }else {
                throw std::runtime_error(std::string("failed to send data") + std::strerror(errno));
            }
        }
    }

    TcpHeader send_header(int socket_fd, const char* file_path) {
        TcpHeader header{};

        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file");
        }
        file.seekg(0, std::ios::end);
        uint64_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        // 先发头部
        /**
         * 文件名长度 ｜ 变长文件名 | 文件长度
         */
        namespace fs = std::filesystem;
        fs::path p = file_path;
        std::string file_name = p.filename().string();
        // 发送文件名长度
        std::array<char, 8> file_name_size_bytes = util::uint64_to_bytes(static_cast<uint64_t>(file_name.size()));
        util::send_all(socket_fd, file_name_size_bytes.data(), 8, 8);

        // 发送文件名
        util::send_all(socket_fd, file_name.c_str(), file_name.size(), 32);

        // 发送文件长度
        std::array<char, 8> bytes{};
        uint64_t file_size = size;
        bytes = util::uint64_to_bytes(file_size);
        util::send_all(socket_fd,bytes.data(), 8, 8);

        header.file_name = file_name;
        header.file_size = file_size;
        return header;
    }
    TcpHeader recv_header(int socket_fd) {
        TcpHeader header{};
        // 依照协议，先收文件名长度，8字节
        std::string file_name_size_str = util::recv_all(socket_fd, 8, 8);

        // 再收变长文件名
        std::array<char, 8> file_name_size_bytes{};
        for (int i = 0; i < 8; i++) {
            file_name_size_bytes[i] = file_name_size_str[i];
        }
        uint64_t file_name_size = util::bytes_to_uint64(file_name_size_bytes);
        std::string file_name = util::recv_all(socket_fd, file_name_size, 32);


        // 再收文件长度 8字节
        std::string file_size_str = util::recv_all(socket_fd, 8, 8);
        std::array<char, 8> bytes{};
        for (int i = 0; i < 8; i++) {
            bytes[i] = file_size_str[i];
        }
        uint64_t file_size = util::bytes_to_uint64(bytes);

        header.file_name = file_name;
        header.file_size = file_size;
        return header;
    }

    void send_file(int socket_fd, const TcpHeader& header, std::ifstream& file, const ProgressFunc& on_progress) {

        char buf[max_read_block_size];
        // 分块读取并发送
        uint64_t sent_total = 0;
        while (file.read(buf, max_read_block_size) || file.gcount() > 0) {
            send_all(socket_fd, buf, file.gcount(), 1024);
            sent_total += file.gcount();
            if (on_progress) on_progress(sent_total, header.file_size);
        }
    }
    void recv_file(int socket_fd, const TcpHeader& header, const ProgressFunc& on_progress) {
        std::string file_name = header.file_name;
        uint64_t file_size = header.file_size;


        // 创建/打开 文件
        std::string new_file_path = R"(../Recv/)" + file_name;
        std::ofstream output(new_file_path, std::ios::binary);
        if (!output) {
            throw std::runtime_error("file path error");
        }

        uint64_t remaining_size = file_size;
        while (remaining_size > 0) {
            long once_write_file_size = remaining_size < max_write_block_size ? static_cast<long>(remaining_size) : max_write_block_size;
            output.write(recv_all(socket_fd, once_write_file_size, 1024).data(), once_write_file_size);
            remaining_size -= once_write_file_size;
            if (on_progress) on_progress(file_size - remaining_size, file_size);
        }
    }
}