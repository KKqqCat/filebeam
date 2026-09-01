cmake_minimum_required(VERSION 4.2)
project(filebeam)

set(CMAKE_CXX_STANDARD 20)

add_executable(filebeam src/main.cpp
        src/Socket.cpp
        src/CliParse.cpp
)
target_include_directories(filebeam PRIVATE src)
