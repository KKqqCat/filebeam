#include <charconv>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <csignal>

#include "Socket.hpp"
#include "util.hpp"

bool should_stop = false;
void handle_sigint(int) {
    should_stop = true;
}

int main() {
    Socket listen_socket;
    try {
        listen_socket.Bind("127.0.0.1", "8080");
        listen_socket.Listen(10);
    }catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        return 3;
    }
    while (!should_stop) {
        std::cout << "accepting..." << std::endl;
        Socket client_socket;
        try {
            client_socket = std::move(listen_socket.Accept());
        }catch (const std::runtime_error& err) {
            std::cerr << err.what() << std::endl;
            continue;
        }

        try {
            util::recv_file(client_socket.fd());
        }catch (const std::runtime_error& err) {
            std::cerr << err.what() << std::endl;
        }

    }

    return 0;
}
