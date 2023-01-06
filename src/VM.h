#ifndef CPPLOX_VM_H
#define CPPLOX_VM_H

#include <memory>
#include "chunk.h"
#include <array>

constexpr const auto STACK_MAX = 256;

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct VM {
    Chunk* chunk;
    uint8_t* ip;
    std::array<Value, STACK_MAX> stack;
    Value* stackTop;

    VM();

    InterpretResult interpret(const char* source);
    InterpretResult run();
    void printValue(Value constant);
    inline uint8_t read_byte();
    inline Value read_constant();

    void push(Value value);
    Value pop();

    template<typename BINARY>
    constexpr auto binary_op(BINARY f) -> void;
};


#endif //CPPLOX_VM_H
