cmake_minimum_required(VERSION 3.24)
project(cpplox)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(cpplox
        src/main.cpp src/chunk.cpp src/Disassembler.cpp src/VM.cpp src/scanner.cpp src/compiler.cpp src/utils.h src/value.cpp src/object.cpp)
target_link_libraries(cpplox ${CONAN_LIBS})


