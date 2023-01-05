cmake_minimum_required(VERSION 3.24)
project(cpplox)

set(CMAKE_CXX_STANDARD 20)

add_executable(cpplox
        main.cpp chunk.cpp Disassembler.cpp)


