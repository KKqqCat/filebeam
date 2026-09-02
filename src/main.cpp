#include <charconv>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <netinet/in.h>
#include <csignal>

#include "Socket.hpp"
#include "util.hpp"

#include "CliParse.hpp"

int main(int argc, char** argv) {

    CliParseRet parse_ret;
    try {
        parse_ret = parse(argc, argv);
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 3;
    }
    Socket connection_socket;
    try {
        if (parse_ret.is_listener) {
            Socket listener_socket;
            listener_socket.Bind(parse_ret.bind_ip_v4, parse_ret.bind_port);
            listener_socket.Listen();
            std::cout << "Waiting for connecting..." << std::endl;
            connection_socket = std::move(listener_socket.Accept());
        }else {
            connection_socket.Connect(parse_ret.target_ip_v4, parse_ret.target_port);
        }
        connection_socket.SetTimeout(30);
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 3;
    }

    // 运行到这里，没有异常，说明连接建立成功了
    std::cout << "succeed in building connection" << std::endl;
    // 开始传输/接收文件
    try {
        uint64_t speed = 0;
        unsigned int rate = 0;
        bool first_flag = true;

        auto last_tick = std::chrono::steady_clock::now();
        auto begin_tick = std::chrono::steady_clock::now();

        util::ProgressFunc on_progress =
            [&first_flag, &last_tick, &speed, &rate, begin_tick](uint64_t done, uint64_t total) {
            auto now_tick = std::chrono::steady_clock::now();
            auto interval = now_tick - last_tick;
            rate = done * 100 / total;
            // 每0.5秒刷新一次

            if (first_flag || rate == 100 || interval.count() >= 500000000) {
                first_flag = false;
                auto dur = now_tick - begin_tick;
                last_tick = now_tick;
                // 计算平均传输速度
                speed = done * 1000000000 / dur.count() / (1024 * 1024);
                std::cout << "\r" <<"transfer progress :" << rate << "%" <<" | "<< speed << " MB/s" << std::flush;
            }
        };

        if (parse_ret.is_listener) {
            const util::TcpHeader header = util::recv_header(connection_socket.fd());
            std::cout << "file name : " << header.file_name << std::endl << "file size : " << header.file_size <<" bytes"<< std::endl;
            util::recv_file(connection_socket.fd(), header, on_progress);
        }else {
            const util::TcpHeader header = util::send_header(connection_socket.fd(), parse_ret.send_file_path.c_str());
            std::cout << "file name : " << header.file_name << std::endl << "file size : " << header.file_size <<" bytes"<< std::endl;
            std::ifstream send_file{parse_ret.send_file_path.c_str(), std::ios::binary};
            util::send_file(connection_socket.fd(), header, send_file, on_progress);
        }

        std::cout << std::endl << "file transfer over" << std::endl;
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 3;
    }

    return 0;
}

