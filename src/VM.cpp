#include "VM.h"
#include "common.h"
#include "Disassembler.h"
#include <functional>
#include "compiler.h"


VM::VM() {
    stackTop = stack.data();
}


InterpretResult VM::interpret(std::string_view source) {
//    chunk = a_chunk;
//    ip = chunk->code.data();

//    return run();
    compile(source);
    return InterpretResult::OK;
}

InterpretResult VM::run() {
//#define READ_BYTE() (*ip++)
//#define READ_CONSTANT() (chunk->constants[READ_BYTE()])
//#define BINARY_OP(op) \
//    do { \
//      Value b = pop(); \
//      Value a = pop(); \
//      push(a op b); \
//    } while (false)



    OP instruction;
    for (;;) {
        if constexpr (DEBUG_TRACE_EXECUTION) {
            fmt::print("{:>10}", " ");
            for (Value *slot = stack.data(); slot < stackTop; ++slot) {
                fmt::print("[ {} ]", *slot);
            }
            fmt::print("\n");
            Disassembler::disassembleInstruction(*chunk, static_cast<int>(ip - chunk->code.data()));
        }
        switch (instruction = static_cast<OP>(read_byte()); instruction) {
            case OP::CONSTANT:
                push(read_constant());
                break;
            case OP::NEGATE:
                push(-pop());
                break;
            case OP::ADD:
                binary_op(std::plus<Value>{});
                break;
            case OP::SUBTRACT:
                binary_op(std::minus<Value>{});
                break;
            case OP::MULTIPLY:
                binary_op(std::multiplies<Value>{});
                break;
            case OP::DIVIDE:
                binary_op(std::divides<Value>{});
                break;
            case OP::RETURN:
                printValue(pop());
                fmt::print("\n");
                return InterpretResult::OK;

        }
    }

//#undef READ_BYTE
//#undef READ_CONSTANT
//#undef BINARY_OP
}

void VM::printValue(Value constant) {
    fmt::print("{}", constant);
}

uint8_t VM::read_byte() {
    return *ip++;
}

Value VM::read_constant() {
    return chunk->constants[read_byte()];
}

void VM::push(Value value) {
    *(stackTop++) = value;
}

Value VM::pop() {
    return *(--stackTop);
}

template<typename BINARY>
constexpr auto VM::binary_op(BINARY fct) -> void {
    auto b = pop();
    auto a = pop();
    push(fct(a,b));
}


