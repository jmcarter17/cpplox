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
            return constantInstruction(chunk, instruction, index);
        case OP::NEGATE:
        case OP::ADD:
        case OP::SUBTRACT:
        case OP::MULTIPLY:
        case OP::DIVIDE:
        case OP::RETURN:
            return simpleInstruction(instruction, index);
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

int Disassembler::constantInstruction(const Chunk &chunk, OP op, int index) {
    auto constant_index = chunk.code[index + 1];
    fmt::print("{} {:>10} '{}'\n", op, constant_index, asNumber(chunk.constants[constant_index]));

    return index + 2;
}
