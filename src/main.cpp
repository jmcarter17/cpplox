#include "chunk.h"
#include "Disassembler.h"

int main() {
    Chunk chunk;
    chunk.writeConstant(1.2, 124);
    chunk.writeChunk(OP::RETURN, 124);

    Disassembler::disassembleChunk(chunk, "test chunk");
}
