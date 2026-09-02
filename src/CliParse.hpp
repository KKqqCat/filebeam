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

// 用法说明。program_name 一般取 argv[0] 的文件名部分。
std::string usage(const std::string& program_name = "filebeam");

// 解析命令行参数。
// 参数非法时抛 std::runtime_error，消息里已包含具体原因，
// 调用方直接打印即可，不需要再拼接说明。
// 传入 -h / --help 或不带参数时，抛出的消息就是完整用法说明。
CliParseRet parse(int argc, char** argv);


#endif //FILEBEAM_CLIPARSE_HPP
