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

#define max_read_block_size (1024 * 1024)
#define max_write_block_size (1024 * 1024)

namespace util {
    inline std::array<char, 8> uint64_to_bytes(uint64_t value) {
        std::array<char, 8> bytes{};
        for (int i = 0; i < 8; i++) {
            bytes[i] = static_cast<char>((value >> ((7-i)*8)) & 0xff);
        }
        return bytes;
    }

    inline uint64_t bytes_to_uint64(std::array<char, 8> bytes) {
        uint64_t value{};
        for (char byte : bytes) {
            value = (value << 8) | static_cast<unsigned char>(byte);
        }
        return value;
    }


    inline std::string recv_all(int socket_fd, uint64_t len, uint64_t once_recv_size = 256) {
        uint64_t remaining_size = len;
        std::string ret;
        ret.reserve(len);

        while (remaining_size > 0) {
            char buf[once_recv_size];
            ssize_t recv_size = recv(socket_fd, buf, once_recv_size, 0);
            if (recv_size > 0) {
                ret.append(buf, recv_size);
                remaining_size -= recv_size;
                std::cout << "recv size : " << recv_size <<" remaining size : " << remaining_size << std::endl;
            }else if (recv_size == 0) {
                std::cout << "connection closed, recv stop" << std::endl;
                return std::string{};
            }else {
                throw std::runtime_error("failed to recv data");
            }
        }
        return ret;
    }

    inline void send_all(int socket_fd, const char* message, uint64_t len, uint64_t once_send_size = 256) {
        uint64_t remaining_size = len;
        uint64_t sent_size = 0;
        while (sent_size < len) {
            const char *buf = message + sent_size;
            // 一次期望发256字节，如果剩余字节不足256，则期望发剩余字节数大小。
            uint64_t block_size = remaining_size < once_send_size ? remaining_size : once_send_size;
            ssize_t send_size = send(socket_fd, buf, block_size, 0);
            if (send_size > 0) {
                // 更新实际已发送字节数和剩余字节数
                sent_size += send_size;
                remaining_size = len - sent_size;
                std::cout << "send_size : " << send_size << ", sent_size : " << sent_size << std::endl;
            }else if (send_size == 0) {
                std::cout << "connection closed, send stop" << std::endl;
                return;
            }else {
                throw std::runtime_error("failed to send data");
            }
        }
    }

    inline void send_file(int socket_fd, const char* file_path) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file");
        }
        file.seekg(0, std::ios::end);
        uint64_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        // 先发头部
        /**
         *  文件名 | 文件长度
         */
        // 发送文件名
        namespace fs = std::filesystem;
        fs::path p = file_path;
        std::string file_name = p.filename().string();
        util::send_all(socket_fd, file_name.c_str(), 16, 16);

        // 发送文件长度
        std::array<char, 8> bytes{};
        uint64_t file_size = size;
        bytes = util::uint64_to_bytes(file_size);
        util::send_all(socket_fd,bytes.data(), 8, 8);

        char buf[max_read_block_size];
        // 分块读取并发送
        while (file.read(buf, max_read_block_size) || file.gcount() > 0) {
            send_all(socket_fd, buf, file.gcount(), 1024);
        }
    }

    inline void recv_file(int socket_fd) {

        // 依照协议，先收文件名，16字节
        std::string file_name = util::recv_all(socket_fd, 16, 16);

        // 再收文件长度 8字节
        std::string file_size_str = util::recv_all(socket_fd, 8, 8);
        std::array<char, 8> bytes{};
        for (int i = 0; i < 8; i++) {
            bytes[i] = file_size_str[i];
        }
        uint64_t file_size = util::bytes_to_uint64(bytes);

        // 创建/打开 文件
        std::string new_file_path = R"(../Recv/)" + file_name;
        std::ofstream output(new_file_path, std::ios::binary);
        if (!output) {
            throw std::runtime_error("file path error");
        }

        uint64_t remaining_size = file_size;
        while (remaining_size > 0) {
            long once_read_file_size = remaining_size < max_write_block_size ? static_cast<long>(file_size) : max_write_block_size;
            output.write(recv_all(socket_fd, file_size, 1024).data(), once_read_file_size);
            remaining_size -= once_read_file_size;
        }
    }

}


#endif //FILEBEAM_UTIL_HPP
