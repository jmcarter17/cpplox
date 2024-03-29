
#ifndef CPPLOX_COMPILER_H
#define CPPLOX_COMPILER_H

#include <string_view>
#include "chunk.h"
#include "scanner.h"
#include "VM.h"
#include "common.h"
#include <type_traits>
#include <map>
#include <cstdint>


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

struct Local {
    Token name;
    int depth{};
};

struct CompilerLocals {
    Local locals[UINT8_COUNT];
    int localCount{};
    int scopeDepth{};
};

class Compiler {
    Parser parser;
    Scanner scanner;
    Chunk *compilingChunk;
    VM *vm;
    CompilerLocals current{};

//    Compiler functions
    void advance();

    void consume(TokenType type, std::string_view message);

    void endCompiler();

    void beginScope();

    void endScope();

    void declareVariable();

    void addLocal(Token name);

    void parsePrecedence(Precedence precedence);

//  Parser functions
    void expression();

    void block();

    void number(bool canAssign);

    void grouping(bool canAssign);

    void unary(bool canAssign);

    void binary(bool canAssign);

    void literal(bool canAssign);

    void and_(bool canAssign);

    void or_(bool canAssign);

    uint8_t makeConstant(Value value);

    void string(bool canAssign);

    void error(std::string_view message);

    void errorAt(const Token &token, std::string_view message);

    void errorAtCurrent(std::string_view message);

    void emitByte(uint8_t byte);

    void emitByte(OP opcode);

    void emitReturn();

    void emitBytes(OP opcode, uint8_t byte);

    void emitConstant(Value value);

    Chunk *currentChunk();

    using ParseFn = void (Compiler::*)(bool canAssign);

    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    std::map<TokenType, ParseRule> rules;

    ParseRule *getRule(TokenType type);

public:
    explicit Compiler(std::string_view source, VM *vm);

    bool compile(Chunk *chunk);

    ObjString *copyString(std::string_view lexeme);

    void declaration();

    void statement();

    void printStatement();

    bool match(TokenType type);

    [[nodiscard]] bool check(TokenType type) const;

    void expressionStatement();

    void synchronize();

    void varDeclaration();

    uint8_t parseVariable(std::string_view message);

    uint8_t identifierConstant(const Token &token);

    static bool identifiersEqual(const Token &a, const Token &b);

    int resolveLocal(Token &name);

    void defineVariable(uint8_t global);

    void variable(bool canAssign);

    void namedVariable(Token name, bool canAssign);

    void markInitialized();

    void ifStatement();

    int emitJump(OP op);

    void patchJump(int offset);

    void whileStatement();

    void emitLoop(unsigned long start);

    void forStatement();
};


#endif //CPPLOX_COMPILER_H
