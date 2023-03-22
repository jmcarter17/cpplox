cmake_minimum_required(VERSION 3.24)
project(cpplox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER /usr/local/bin/g++)
set(CMAKE_C_COMPILER /usr/local/bin/gcc)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
# conan_basic_setup()

find_package(fmt REQUIRED)
find_package(magic_enum REQUIRED)

add_executable(cpplox
        src/main.cpp src/chunk.cpp src/Disassembler.cpp src/VM.cpp src/scanner.cpp src/compiler.cpp src/utils.h src/value.cpp)
target_link_libraries(cpplox fmt::fmt magic_enum::magic_enum)


