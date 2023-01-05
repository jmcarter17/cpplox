#include "chunk.h"
#include "Disassembler.h"

int main() {
    Chunk chunk;
    size_t constant = chunk.addConstant(1.2);
    chunk.writeChunk(OP::CONSTANT, 124);
    chunk.writeChunk(constant, 124);

    chunk.writeChunk(OP::RETURN, 125);

    Disassembler::disassembleChunk(chunk, "test chunk");
}
