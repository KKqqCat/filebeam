//
// Created by Singleton on 2026/8/31.
//

#include "CliParse.hpp"

#include <arpa/inet.h>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <system_error>

namespace {
    namespace fs = std::filesystem;

    // 端口 0 的含义是"由内核分配"，对本程序来说总是误输入，因此下界取 1
    constexpr unsigned long port_min = 1;
    constexpr unsigned long port_max = 65535;

    // 下面几个 check_ 函数返回空字符串表示合法，否则返回具体原因。
    // 这样校验逻辑不需要知道程序名，抛异常的动作统一收在 parse 里。

    std::string check_ipv4(const std::string& ip) {
        if (ip.empty()) {
            return "IP 地址为空";
        }
        // inet_pton 只接受严格的点分十进制四段格式，
        // "1.2.3"、"1.2.3.4.5"、"1.2.3.256" 都会被拒绝
        in_addr addr{};
        if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
            return "不是合法的 IPv4 地址: '" + ip + "'";
        }
        return {};
    }

    std::string check_port(const std::string& port) {
        if (port.empty()) {
            return "端口为空";
        }
        unsigned long value = 0;
        const char* begin = port.data();
        const char* end = port.data() + port.size();
        const auto [ptr, ec] = std::from_chars(begin, end, value);
        // ptr != end 说明有尾部残留，"8080x" 这类要拒绝
        if (ec != std::errc{} || ptr != end) {
            return "端口必须是十进制数字: '" + port + "'";
        }
        if (value < port_min || value > port_max) {
            return "端口超出 " + std::to_string(port_min) + "-" + std::to_string(port_max)
                   + " 范围: '" + port + "'";
        }
        return {};
    }

    // 把 "ip:port" 拆开。用最后一个冒号定位，
    // 这样 IPv6 形式能落到"冒号多于一个"的分支给出明确提示，而不是被错误地拆分。
    std::string split_ip_port(const std::string& text, std::string& ip, std::string& port) {
        const size_t pos = text.rfind(':');
        if (pos == std::string::npos) {
            return "地址缺少端口，应为 <ip>:<port> 形式: '" + text + "'";
        }
        ip = text.substr(0, pos);
        port = text.substr(pos + 1);

        if (ip.find(':') != std::string::npos) {
            return "本程序只支持 IPv4，无法解析: '" + text + "'";
        }
        if (auto err = check_ipv4(ip); !err.empty()) {
            return err;
        }
        return check_port(port);
    }

    std::string check_send_path(const std::string& path) {
        if (path.empty()) {
            return "待发送的文件路径为空";
        }

        const fs::path p{path};
        // 协议只传 filename() 部分，取不出文件名的话接收端无法落地
        const fs::path name = p.filename();
        if (name.empty() || name == "." || name == "..") {
            return "无法从路径中取出文件名: '" + path + "'";
        }

        std::error_code ec;
        if (!fs::exists(p, ec) || ec) {
            return "文件不存在: '" + path + "'";
        }
        if (!fs::is_regular_file(p, ec) || ec) {
            return "不是普通文件（目录或设备？）: '" + path + "'";
        }
        // 存在但打不开（权限不足）在这里就报出来，比传输开始后再失败清楚
        std::ifstream probe{p, std::ios::binary};
        if (!probe) {
            return "文件无法读取，检查权限: '" + path + "'";
        }
        return {};
    }

    [[noreturn]] void fail(const std::string& program_name, const std::string& reason) {
        throw std::runtime_error(
            program_name + ": " + reason + "\n运行 " + program_name + " --help 查看用法");
    }
}

std::string usage(const std::string& program_name) {
    return
        "用法:\n"
        "  " + program_name + " recv <bind_ip>:<port>\n"
        "  " + program_name + " send <target_ip>:<port> <file_path>\n"
        "\n"
        "子命令:\n"
        "  recv   监听指定地址和端口，接收一个文件并保存到 ../Recv/ 下\n"
        "         （保存目录相对于当前工作目录，需要预先存在）\n"
        "  send   连接目标地址和端口，发送一个文件\n"
        "\n"
        "参数:\n"
        "  <bind_ip>      本机监听地址，IPv4，0.0.0.0 表示所有网卡\n"
        "  <target_ip>    目标主机地址，IPv4\n"
        "  <port>         端口，" + std::to_string(port_min) + "-" + std::to_string(port_max) + "\n"
        "  <file_path>    待发送文件的路径，必须是可读的普通文件\n"
        "\n"
        "示例:\n"
        "  " + program_name + " recv 0.0.0.0:8080\n"
        "  " + program_name + " send 192.168.1.10:8080 ./Send/photo.jpg\n"
        "\n"
        "选项:\n"
        "  -h, --help     显示本说明\n";
}

CliParseRet parse(int argc, char** argv) {
    const std::string program_name =
        (argc > 0 && argv[0] != nullptr && argv[0][0] != '\0')
            ? fs::path(argv[0]).filename().string()
            : "filebeam";

    // 不带参数时直接给用法，比报一句"参数个数错误"有用
    if (argc < 2) {
        throw std::runtime_error(usage(program_name));
    }

    const std::string mode = argv[1];
    if (mode == "-h" || mode == "--help") {
        throw std::runtime_error(usage(program_name));
    }

    CliParseRet ret;
    if (mode == "send") {
        if (argc != 4) {
            fail(program_name, "send 需要 2 个参数（<ip>:<port> 和 <file_path>），实际收到 "
                               + std::to_string(argc - 2) + " 个");
        }
        if (auto err = split_ip_port(argv[2], ret.target_ip_v4, ret.target_port); !err.empty()) {
            fail(program_name, err);
        }
        if (auto err = check_send_path(argv[3]); !err.empty()) {
            fail(program_name, err);
        }
        ret.is_listener = false;
        ret.send_file_path = argv[3];
    } else if (mode == "recv") {
        if (argc != 3) {
            fail(program_name, "recv 需要 1 个参数（<ip>:<port>），实际收到 "
                               + std::to_string(argc - 2) + " 个");
        }
        if (auto err = split_ip_port(argv[2], ret.bind_ip_v4, ret.bind_port); !err.empty()) {
            fail(program_name, err);
        }
        ret.is_listener = true;
    } else {
        fail(program_name, "未知的子命令 '" + mode + "'，应为 send 或 recv");
    }

    return ret;
}
