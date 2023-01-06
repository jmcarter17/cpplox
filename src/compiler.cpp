#include "compiler.h"
#include "scanner.h"
#include <fmt/core.h>
#include <magic_enum.hpp>


void compile(const char *source) {
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
        printf("%2d '%.*s'\n", magic_enum::enum_integer(token.type), token.length, token.start);
    }
}