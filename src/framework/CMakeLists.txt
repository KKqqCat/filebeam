cmake_minimum_required(VERSION 4.2)

add_library(netbase core.cpp
        Socket.cpp
)

target_include_directories(netbase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)