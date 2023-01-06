cmake_minimum_required(VERSION 3.24)
project(cpplox)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(cpplox
        src/main.cpp src/chunk.cpp src/Disassembler.cpp src/VM.cpp src/VM.h)
target_link_libraries(cpplox ${CONAN_LIBS})


