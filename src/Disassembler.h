#ifndef CPPLOX_DISASSEMBLER_H
#define CPPLOX_DISASSEMBLER_H


#include <string_view>
#include "chunk.h"

struct Disassembler {
    static void disassembleChunk(const Chunk& chunk, std::string_view name);
    static int disassembleInstruction(const Chunk& chunk, int index);

    static int simpleInstruction(OP code, int index);

    static int constantInstruction(const Chunk& chunk, OP op, int index);

    static int unknownInstruction(OP op, int index);
};


#endif //CPPLOX_DISASSEMBLER_H
