cmake_minimum_required(VERSION 3.8)
project(ssh_exec)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(ssh_exec ${SOURCE_FILES})

target_link_libraries(ssh_exec ssh2)
