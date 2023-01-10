#include "VM.h"
#include "common.h"
#include "Disassembler.h"
#include <functional>
#include "compiler.h"
#include <cstdarg>


VM::VM() : stackTop{stack.data()} {}

InterpretResult VM::interpret(std::string_view source) {
    auto* a_chunk = new Chunk{};
    Compiler compiler(source);

    if (!compiler.compile(a_chunk)) {
        return InterpretResult::COMPILE_ERROR;
    }

    this->chunk = a_chunk;
    this->ip = a_chunk->code.data();

    InterpretResult result = run();
    return result;
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
                fmt::print("[ {} ]", asNumber(*slot));
            }
            fmt::print("\n");
            Disassembler::disassembleInstruction(*chunk, static_cast<int>(ip - chunk->code.data()));
        }
        switch (instruction = static_cast<OP>(read_byte()); instruction) {
            case OP::CONSTANT:
                push(read_constant());
                break;
            case OP::NEGATE:
                if (!isNumber(peek(0))){
                    runtimeError(fmt::runtime("Operand must be a number."));
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(number_val(-(asNumber(pop()))));
                break;
            case OP::ADD:
//                auto result = binary_op(std::plus<double>{});
                if (binary_op(std::plus<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
                break;
            case OP::SUBTRACT:
                if (binary_op(std::minus<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
                break;
            case OP::MULTIPLY:
                if (binary_op(std::multiplies<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
                break;
            case OP::DIVIDE:
                if (binary_op(std::divides<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
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
    fmt::print("{}", asNumber(constant));
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

Value VM::peek(int distance) {
    return stackTop[-1 - distance];
}

template<typename... Args>
void VM::runtimeError(fmt::basic_runtime<char> format, Args &&... args) {
    fmt::print(stderr, format, args...);
    fmt::print(stderr,"\n");

    size_t instruction = ip - chunk->code.data() - 1;
    int line = chunk->lines[instruction].line_no;
    fmt::print(stderr, "[line {}] in script\n", line);
    resetStack();
}

void VM::resetStack() {
    stackTop = stack.data();
}

template<typename BINARY>
constexpr auto VM::binary_op(BINARY fct) -> InterpretResult {
    if (!isNumber(peek(0)) || !isNumber(peek(1))) {
        runtimeError(fmt::runtime("Operands must be numbers."));
        return InterpretResult::RUNTIME_ERROR;
    }
    double b = asNumber(pop());
    double a = asNumber(pop());
    push(number_val(fct(a, b)));
    return InterpretResult::OK;
}


