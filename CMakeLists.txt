cmake_minimum_required(VERSION 4.2)
project(filebeam)

set(CMAKE_CXX_STANDARD 20)

add_executable(filebeam main.cpp
        Socket.cpp
        Socket.hpp)
add_executable(client client.cpp
        Socket.cpp
        Socket.hpp
        util.hpp
)