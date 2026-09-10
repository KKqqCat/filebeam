//
// Created by Singleton on 2026/9/7.
//

#ifndef FILEBEAM_UTIL_HPP
#define FILEBEAM_UTIL_HPP
#include <array>
#include <cstdint>

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
}
#endif //FILEBEAM_UTIL_HPP
