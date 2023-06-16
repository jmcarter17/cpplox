#include <iostream>
#include <iomanip>
#include "Disassembler.h"
#include <fmt/core.h>

void Disassembler::disassembleChunk(const Chunk &chunk, std::string_view name) {
    fmt::print("== {} ==\n", name);

    for (int idx = 0; idx < chunk.code.size();) {
        idx = disassembleInstruction(chunk, idx);
    }
}

int Disassembler::disassembleInstruction(const Chunk &chunk, int index) {
    fmt::print("{:#04} ", index);
    if (index > 0 && chunk.getLine(index) == chunk.getLine(index - 1)) {
        fmt::print("{:>5}", "| ");
    } else {
        fmt::print("{:>4} ", chunk.getLine(index));
    }

    switch (auto instruction = static_cast<OP>(chunk.code[index]); instruction) {
        case OP::CONSTANT:
        case OP::DEFINE_GLOBAL:
        case OP::GET_GLOBAL:
        case OP::SET_GLOBAL:
            return constantInstruction(chunk, instruction, index);
        case OP::GET_LOCAL:
        case OP::SET_LOCAL:
            return byteInstruction(chunk, instruction, index);
        case OP::NIL:
        case OP::TRUE:
        case OP::FALSE:
        case OP::NEGATE:
        case OP::EQUAL:
        case OP::NOT_EQUAL:
        case OP::GREATER:
        case OP::GREATER_EQUAL:
        case OP::LESS:
        case OP::LESS_EQUAL:
        case OP::ADD:
        case OP::SUBTRACT:
        case OP::MULTIPLY:
        case OP::DIVIDE:
        case OP::NOT:
        case OP::PRINT:
        case OP::POP:
        case OP::RETURN:
            return simpleInstruction(instruction, index);
        case OP::JUMP:
        case OP::JUMP_IF_TRUE:
        case OP::JUMP_IF_FALSE:
            return jumpInstruction(chunk, instruction, 1, index);
        case OP::LOOP:
            return jumpInstruction(chunk, instruction, -1, index);
        default:
            return unknownInstruction(instruction, index);
    }
}

int Disassembler::unknownInstruction(OP op, int index) {
    fmt::print("Unknown instruction at index {}", index);
    return index + 1;
}

int Disassembler::simpleInstruction(OP code, int index) {
    fmt::print("{}\n", code);
    return index + 1;
}

int Disassembler::byteInstruction(const Chunk &chunk, OP code, int index) {
    auto slot = chunk.code[index + 1];
    fmt::print("{} {}\n", code, slot);
    return index + 2;
}

int Disassembler::constantInstruction(const Chunk &chunk, OP op, int index) {
    auto constant_index = chunk.code[index + 1];
    fmt::print("{} {} ", op, constant_index);
    printValue(chunk.constants[constant_index]);
    fmt::print("\n");
    return index + 2;
}

int Disassembler::jumpInstruction(const Chunk &chunk, OP op, int sign, int index) {
    uint16_t jump = static_cast<uint16_t>(chunk.code[index + 1] << 8) | chunk.code[index + 2];
    fmt::print("{} {} {}\n", op, index, index + 3 + sign * jump);

    return index + 3;
}
