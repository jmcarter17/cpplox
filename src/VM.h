#ifndef CPPLOX_VM_H
#define CPPLOX_VM_H

#include <memory>
#include "chunk.h"
#include <array>
#include <unordered_set>
#include <unordered_map>

constexpr const auto STACK_MAX = 256;

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

struct VM {
    Chunk* chunk{};
    uint8_t* ip{};
    std::array<Value, STACK_MAX> stack;
    Value* stackTop;
    Obj* objects{nullptr};
    std::unordered_set<std::string> strings;
    std::unordered_map<std::string, Value> globals;

    VM();
    ~VM();

    InterpretResult interpret(std::string_view);
    InterpretResult run();
    inline uint8_t read_byte();
    inline uint16_t read_short();
    inline Value read_constant();

    void push(Value value);
    Value pop();

    template<typename BINARY>
    constexpr auto binary_op(BINARY fct) -> InterpretResult;

    Value peek(int distance) const;

    template<typename... Args>
    void runtimeError(fmt::basic_runtime<char> format, Args&&... args);

    void resetStack();

    void concatenate();

    void deleteObjects() const;

    ObjString *read_string();
};


#endif //CPPLOX_VM_H
