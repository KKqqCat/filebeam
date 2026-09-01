#include <charconv>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
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
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 3;
    }

    // 运行到这里，没有异常，说明连接建立成功了
    std::cout << "succeed in building connection" << std::endl;
    // 开始传输/接收文件
    try {
        if (parse_ret.is_listener) {
            util::recv_file(connection_socket.fd());
        }else {
            util::send_file(connection_socket.fd(), parse_ret.send_file_path.c_str());
        }
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 3;
    }

    return 0;
}

