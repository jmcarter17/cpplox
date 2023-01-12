#include "VM.h"
#include "common.h"
#include "Disassembler.h"
#include <functional>
#include "compiler.h"
#include <cstdarg>


VM::VM() : stackTop{stack.data()} {}

InterpretResult VM::interpret(std::string_view source) {
    auto *a_chunk = new Chunk{};
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
                fmt::print("[ ");
                printValue(*slot);
                fmt::print(" ]");
            }
            fmt::print("\n");
            Disassembler::disassembleInstruction(*chunk, static_cast<int>(ip - chunk->code.data()));
        }
        switch (instruction = static_cast<OP>(read_byte()); instruction) {
            case OP::CONSTANT:
                push(read_constant());
                break;
            case OP::NIL:
                push(nil_val());
                break;
            case OP::TRUE:
                push(bool_val(true));
                break;
            case OP::FALSE:
                push(bool_val(false));
                break;
            case OP::NEGATE:
                if (!isNumber(peek(0))) {
                    runtimeError(fmt::runtime("Operand must be a number."));
                    return InterpretResult::RUNTIME_ERROR;
                }
                push(number_val(-(asNumber(pop()))));
                break;
            case OP::EQUAL: {
                Value b = pop();
                Value a = pop();
                push(bool_val(a == b));
                break;
            }
            case OP::NOT_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(bool_val(a != b));
                break;
            }
            case OP::GREATER: {
                if (binary_op(std::greater<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
                break;
            }
            case OP::GREATER_EQUAL: {
                if (binary_op(std::greater_equal<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
                break;
            }
            case OP::LESS: {
                if (binary_op(std::less<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
                break;
            }
            case OP::LESS_EQUAL: {
                if (binary_op(std::less_equal<double>{}) == InterpretResult::RUNTIME_ERROR)
                    return InterpretResult::RUNTIME_ERROR;
                break;
            }
            case OP::ADD:
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
            case OP::NOT:
                push(bool_val(isFalsey(pop())));
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
    fmt::print(stderr, "\n");

    size_t instruction = ip - chunk->code.data() - 1;
//    int line = chunk->lines[instruction].line_no;
    int line = chunk->getLine(instruction);
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
    push(createValue(fct(a, b)));
    return InterpretResult::OK;
}


