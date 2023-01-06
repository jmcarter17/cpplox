#include "chunk.h"
#include "Disassembler.h"
#include "VM.h"
#include <fmt/core.h>

static void repl() {
    VM vm;
    char line[1024];
    for (;;) {
        fmt::print("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            fmt::print("\n");
            break;
        }

        vm.interpret(line);
    }
}

static char* readFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == nullptr) {
        fmt::print(stderr, "Could not open file \"{}\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == nullptr) {
        fmt::print(stderr, "Not enough memory to read \"{}\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fmt::print(stderr, "Could not read file \"{}\".\n", path);
        exit(74);
    }
    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

static void runFile(const char* path) {
    VM vm;
    const char* source{readFile(path)};
    InterpretResult result = vm.interpret(source);
//    free(source);

    if (result == InterpretResult::COMPILE_ERROR) exit(65);
    if (result == InterpretResult::RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]) {
    VM vm{};

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }

//    Chunk chunk;
//    chunk.writeConstant(1.2, 124);
//    chunk.writeConstant(3.4, 124);
//    chunk.writeChunk(OP::ADD, 124);
//    chunk.writeConstant(5.6, 124);
//    chunk.writeChunk(OP::DIVIDE, 124);
//    chunk.writeChunk(OP::NEGATE, 124);
//    chunk.writeChunk(OP::RETURN, 125);
//
//    Disassembler::disassembleChunk(chunk, "test chunk");

//    auto result = vm.interpret(&chunk);
//    fmt::print("{}", magic_enum::enum_name(result));
}
