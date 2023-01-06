#include "chunk.h"
#include "Disassembler.h"
#include "VM.h"

int main() {
    VM vm;

    Chunk chunk;
    chunk.writeConstant(1.2, 124);
    chunk.writeConstant(3.4, 124);
    chunk.writeChunk(OP::ADD, 124);
    chunk.writeConstant(5.6, 124);
    chunk.writeChunk(OP::DIVIDE, 124);
    chunk.writeChunk(OP::NEGATE, 124);
    chunk.writeChunk(OP::RETURN, 125);

    Disassembler::disassembleChunk(chunk, "test chunk");

    auto result = vm.interpret(&chunk);
//    fmt::print("{}", magic_enum::enum_name(result));
}
