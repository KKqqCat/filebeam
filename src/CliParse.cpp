//
// Created by Singleton on 2026/8/31.
//

#include "CliParse.hpp"
#include <stdexcept>

/**
 * filebeam [send|recv] [ip(v4):port] [send_file_path]
 *
 */
CliParseRet parse(int argc, char **argv) {
    if (argc < 3) {
        throw std::runtime_error("cli parse err : argc err");
    }
    CliParseRet ret;
    std::string argv_1 = argv[1];
    if (argv_1 == "send") {
        if (argc != 4) {
            throw std::runtime_error("cli parse err, argc err");
        }
        std::string ip_port = argv[2];
        size_t pos = ip_port.find(":", 0);
        if (pos == std::string::npos) {
            throw std::runtime_error("cli parse err: ip_port parse err");
        }
        ret.is_listener = false;
        ret.target_ip_v4 = ip_port.substr(0, pos);
        ret.target_port = ip_port.substr(pos+1, ip_port.size()-pos-1);
        ret.send_file_path = argv[3];

    }else if (argv_1 == "recv") {
        if (argc != 3) {
            throw std::runtime_error("cli parse err, argc err");
        }
        std::string ip_port = argv[2];
        size_t pos = ip_port.find(":", 0);
        if (pos == std::string::npos) {
            throw std::runtime_error("cli parse err: ip_port parse err");
        }
        ret.is_listener = true;
        ret.bind_ip_v4 = ip_port.substr(0, pos);
        ret.bind_port = ip_port.substr(pos+1, ip_port.size()-pos-1);
    }else {
        throw std::runtime_error("cli parse err");
    }
    return ret;
}
