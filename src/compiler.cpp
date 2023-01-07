#include "compiler.h"
#include "scanner.h"
#include <fmt/core.h>


void compile(std::string_view source) {
    auto scanner = Scanner(source);
    int line = -1;
    for (;;) {
        Token token = scanner.scanToken();
        if (token.type == TokenType::EOFILE) {
            break;
        }
        if (token.line != line) {
            fmt::print("{:>4} ", token.line);
            line = token.line;
        } else {
            fmt::print("{:>5}", "| ");
        }
        fmt::print("{} '{}'\n", token.type, token.lexeme);
    }
}