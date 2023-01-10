
#ifndef CPPLOX_COMPILER_H
#define CPPLOX_COMPILER_H

#include <string_view>
#include "chunk.h"
#include "scanner.h"
#include <type_traits>
#include <map>


enum class Precedence {
    NONE,
    ASSIGNMENT,  // =
    OR,          // or
    AND,         // and
    EQUALITY,    // == !=
    COMPARISON,  // < > <= >=
    TERM,        // + -
    FACTOR,      // * /
    UNARY,       // ! -
    CALL,        // . ()
    PRIMARY
};


struct Parser {
    Token current{};
    Token previous{};
    bool hadError{};
    bool panicMode{};
};

class Compiler {
    Parser parser;
    Scanner scanner;
    Chunk *compilingChunk;

//    Compiler functions
    void advance();
    void consume(TokenType type, std::string_view message);
    void endCompiler();
    void parsePrecedence(Precedence precedence);

//    expressions
    void expression();
    void number();
    void grouping();
    void unary();
    void binary();
    uint8_t makeConstant(Value value);

    void error(std::string_view message);
    void errorAt(const Token &token, std::string_view message);
    void errorAtCurrent(std::string_view message);

    void emitByte(uint8_t byte);
    void emitByte(OP opcode);
    void emitReturn();
    void emitBytes(OP opcode, uint8_t byte);
    void emitConstant(Value value);

    Chunk *currentChunk();

//    typedef void (Compiler::*ParseFn)();
    using ParseFn = void (Compiler::*)();

    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    std::map<TokenType, ParseRule> rules;
    ParseRule *getRule(TokenType type);

public:
    explicit Compiler(std::string_view source);
    bool compile(Chunk *chunk);
};


#endif //CPPLOX_COMPILER_H
