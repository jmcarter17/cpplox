#include "chunk.h"
#include "magic_enum.hpp"

auto Chunk::writeChunk(uint8_t opcode, int line) -> void {
    code.push_back(opcode);
    if (!lines.empty() and lines.back().line_no == line) {
        lines.back().num_instructions += 1;
    } else {
        lines.emplace_back(line);
    }
}

auto Chunk::addConstant(Value value) -> uint8_t {
    constants.push_back(value);
    return static_cast<uint8_t>(constants.size() - 1);
}

auto Chunk::getLine(size_t index) const -> int {
    int total = 0;
    for (auto info: lines) {
        total += info.num_instructions;
        if (total > index) {
            return info.line_no;
        }
    }
    return -1;
}