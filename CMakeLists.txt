cmake_minimum_required(VERSION 3.10)

project(cpp_server)

set(CMAKE_CXX_STANDARD 17)

# 头文件路径
include_directories(include)

# 可执行文件
add_executable(server
    src/main.cpp
    src/reactor.cpp
    src/connection.cpp
    src/utils.cpp
    src/thread_pool.cpp
)