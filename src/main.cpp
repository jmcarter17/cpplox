#include "chunk.h"
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
    std::unique_ptr<char> source{readFile(path)};
    InterpretResult result = vm.interpret(std::string_view{source.get()});

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
        fmt::print(stderr, "Usage: clox [path]\n");
        exit(64);
    }
}
