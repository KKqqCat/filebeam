cmake_minimum_required(VERSION 4.2)
project(filebeam)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(src/framework)
add_executable(filebeam
        src/application/main.cpp
        src/application/CliParse.cpp
        src/application/FileTransfer.cpp
)
target_include_directories(filebeam PRIVATE src)
target_link_libraries(filebeam PRIVATE netbase)