//
// Created by Singleton on 2026/8/31.
//

#ifndef FILEBEAM_CLIPARSE_HPP
#define FILEBEAM_CLIPARSE_HPP
#include <string>

struct CliParseRet {
    bool is_listener = true;
    std::string bind_ip_v4 {};
    std::string bind_port {};
    std::string target_ip_v4 {};
    std::string target_port {};
    std::string send_file_path {};
};

CliParseRet parse(int argc, char** argv);


#endif //FILEBEAM_CLIPARSE_HPP
