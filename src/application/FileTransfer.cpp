//
// Created by Singleton on 2026/9/7.
//

#include "FileTransfer.hpp"
#include "framework/core.hpp"
#include "util.hpp"

#define max_read_block_size (1024 * 1024)
#define max_write_block_size (1024 * 1024)

using namespace Netbase;

namespace FileTransfer {
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
        core::send_all(socket_fd, file_name_size_bytes.data(), 8, 8);

        // 发送文件名
        core::send_all(socket_fd, file_name.c_str(), file_name.size(), 32);

        // 发送文件长度
        std::array<char, 8> bytes{};
        uint64_t file_size = size;
        bytes = util::uint64_to_bytes(file_size);
        core::send_all(socket_fd,bytes.data(), 8, 8);

        header.file_name = file_name;
        header.file_size = file_size;
        return header;
    }
    TcpHeader recv_header(int socket_fd) {
        TcpHeader header{};
        // 依照协议，先收文件名长度，8字节
        std::string file_name_size_str = core::recv_all(socket_fd, 8, 8);

        // 再收变长文件名
        std::array<char, 8> file_name_size_bytes{};
        for (int i = 0; i < 8; i++) {
            file_name_size_bytes[i] = file_name_size_str[i];
        }
        uint64_t file_name_size = util::bytes_to_uint64(file_name_size_bytes);
        std::string file_name = core::recv_all(socket_fd, file_name_size, 32);


        // 再收文件长度 8字节
        std::string file_size_str = core::recv_all(socket_fd, 8, 8);
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
            core::send_all(socket_fd, buf, file.gcount(), 1024);
            sent_total += file.gcount();
            if (on_progress) on_progress(sent_total, header.file_size);
        }
    }
    void recv_file(int socket_fd, const TcpHeader& header, const ProgressFunc& on_progress) {
        std::string file_name = header.file_name;
        uint64_t file_size = header.file_size;

        if (!std::filesystem::is_directory(R"(Recv)")) {
            std::filesystem::create_directory(R"(Recv)");
        }

        // 创建/打开 文件
        std::string new_file_path = R"(Recv/)" + file_name;
        std::ofstream output(new_file_path, std::ios::binary);
        if (!output) {
            throw std::runtime_error("file path error");
        }

        uint64_t remaining_size = file_size;
        while (remaining_size > 0) {
            long once_write_file_size = remaining_size < max_write_block_size ? static_cast<long>(remaining_size) : max_write_block_size;
            output.write(core::recv_all(socket_fd, once_write_file_size, 1024).data(), once_write_file_size);
            remaining_size -= once_write_file_size;
            if (on_progress) on_progress(file_size - remaining_size, file_size);
        }
    }
} // FileTransfer