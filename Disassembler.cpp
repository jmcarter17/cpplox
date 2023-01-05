#include <iostream>
#include <iomanip>
#include "Disassembler.h"

void Disassembler::disassembleChunk(const Chunk &chunk, std::string_view name) {
    std::cout << "== " << name << " ==\n";

    for (int idx = 0; idx < chunk.code.size();) {
        idx = disassembleInstruction(chunk, idx);
    }
}

int Disassembler::disassembleInstruction(const Chunk &chunk, int index) {
    std::cout << std::setfill('0') << std::setw(4) << index << ' ';
    if (index > 0 && chunk.getLine(index) == chunk.getLine(index - 1)) {
        std::cout << "   | ";
    } else {
        std::cout << std::setw(4) << chunk.getLine(index) << ' ';
    }

    auto instruction = static_cast<OP>(chunk.code[index]);
    switch (instruction) {
        case OP::RETURN:
            return simpleInstruction(instruction, index);
        case OP::CONSTANT:
            return constantInstruction(chunk, instruction, index);
        default:
            std::cout << "Unknown opcode " << instruction << '\n';
            return index + 1;
    }
}

int Disassembler::simpleInstruction(OP code, int index) {
    std::cout << code << '\n';
    return index + 1;
}

int Disassembler::constantInstruction(const Chunk& chunk, OP op, int index) {
    auto constant_index = chunk.code[index+1];
    std::cout << op << " " << static_cast<int>(constant_index) << ' ';
    std::cout << chunk.constants[constant_index] << '\n';
    return index + 2;
}
