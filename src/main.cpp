#include "chunk.h"
#include "Disassembler.h"

int main() {
    Chunk chunk;
    size_t constant = chunk.addConstant(1.2);
    chunk.writeChunk(to_integral(OP::CONSTANT), 124);
    chunk.writeChunk(constant, 124);

    chunk.writeChunk(to_integral(OP::RETURN), 124);
    chunk.writeChunk(23, 126);

    Disassembler::disassembleChunk(chunk, "test chunk");
}
