//
// Created by Singleton on 2026/7/30.
//
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Socket.hpp"
#include "util.hpp"
#include <fstream>

int main() {
    Socket client_socket;

    try {
        client_socket.Connect("127.0.0.1", "8080");
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 3;
    }

    std::cout << "connect success!" << std::endl;
    try {
        util::send_file(client_socket.fd(), R"(../Send/sendimgTest.jpg)");
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
    }
    std::cout << "connect close" << std::endl;
    return 0;
}