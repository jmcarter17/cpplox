#include "compiler.h"
#include "scanner.h"
#include <fmt/core.h>
#include "common.h"
#include "Disassembler.h"

Compiler::Compiler(std::string_view source) :
    scanner{source}, parser{}, rules{
        [to_integral(TokenType::LEFT_PAREN)]    = {&Compiler::grouping, nullptr, Precedence::NONE},
        [to_integral(TokenType::RIGHT_PAREN)]   = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::LEFT_BRACE)]    = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::RIGHT_BRACE)]   = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::COMMA)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::DOT)]           = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::MINUS)]         = {&Compiler::unary, &Compiler::binary, Precedence::TERM},
        [to_integral(TokenType::PLUS)]          = {nullptr, &Compiler::binary, Precedence::TERM},
        [to_integral(TokenType::SEMICOLON)]     = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::SLASH)]         = {nullptr, &Compiler::binary, Precedence::FACTOR},
        [to_integral(TokenType::STAR)]          = {nullptr, &Compiler::binary, Precedence::FACTOR},
        [to_integral(TokenType::BANG)]          = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::BANG_EQUAL)]    = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::EQUAL)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::EQUAL_EQUAL)]   = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::GREATER)]       = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::GREATER_EQUAL)] = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::LESS)]          = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::LESS_EQUAL)]    = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::IDENTIFIER)]    = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::STRING)]        = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::NUMBER)]        = {&Compiler::number, nullptr, Precedence::NONE},
        [to_integral(TokenType::AND)]           = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::CLASS)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::ELSE)]          = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::FALSE)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::FOR)]           = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::FUN)]           = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::IF)]            = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::NIL)]           = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::OR)]            = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::PRINT)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::RETURN)]        = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::SUPER)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::THIS)]          = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::TRUE)]          = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::VAR)]           = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::WHILE)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::ERROR)]         = {nullptr, nullptr, Precedence::NONE},
        [to_integral(TokenType::EOFILE)]        = {nullptr, nullptr, Precedence::NONE},
    } {}

bool Compiler::compile(Chunk *chunk) {
    compilingChunk = chunk;
    advance();
    expression();
    consume(TokenType::EOFILE, "Expect end of expression");
    endCompiler();
    return !parser.hadError;
}

void Compiler::consume(TokenType type, std::string_view message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

void Compiler::advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanner.scanToken();
        if (parser.current.type != TokenType::ERROR) break;

        errorAtCurrent(parser.current.lexeme);
    }
}

void Compiler::error(std::string_view message) {
    errorAt(parser.previous, message);
}

void Compiler::errorAt(const Token &token, std::string_view message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token.line);

    if (token.type == TokenType::EOFILE) {
        fprintf(stderr, " at end");
    } else if (token.type == TokenType::ERROR) {
        // Nothing.
    } else {
        fmt::print(stderr, " at {}", token.lexeme);
    }

    fmt::print(stderr, ": {}\n", message);
    parser.hadError = true;
}

void Compiler::errorAtCurrent(std::string_view message) {
    errorAt(parser.current, message);
}

Chunk *Compiler::currentChunk() {
    return compilingChunk;
}

void Compiler::emitByte(uint8_t byte) {
    currentChunk()->writeChunk(byte, parser.previous.line);
}

void Compiler::emitByte(OP opcode) {
    emitByte(to_integral(opcode));
}

void Compiler::emitReturn() {
    emitByte(OP::RETURN);
}

void Compiler::emitBytes(OP opcode, uint8_t byte) {
    emitByte(opcode);
    emitByte(byte);
}

void Compiler::emitConstant(double value) {
    emitBytes(OP::CONSTANT, makeConstant(value));
}

void Compiler::endCompiler() {
    emitReturn();
    if constexpr (DEBUG_PRINT_CODE) {
        if (!parser.hadError) {
            Disassembler::disassembleChunk(*currentChunk(), "Code");
        }
    }
}

void Compiler::expression() {
    parsePrecedence(Precedence::ASSIGNMENT);
}

void Compiler::number() {
    auto lexeme = parser.previous.lexeme;
    auto value = std::strtod(lexeme.data(), nullptr);
    emitConstant(value);
}

void Compiler::grouping() {
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect '_' after expression.");
}

void Compiler::unary() {
    TokenType operatorType = parser.previous.type;

    // compile the operand
    parsePrecedence(Precedence::UNARY);

    switch (operatorType) {
        case TokenType::MINUS: emitByte(OP::NEGATE); break;
        default: return; // unreachable
    }
}

void Compiler::binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(to_integral(rule->precedence) + 1));

    switch (operatorType) {
        case TokenType::PLUS: emitByte(OP::ADD); break;
        case TokenType::MINUS: emitByte(OP::SUBTRACT); break;
        case TokenType::STAR: emitByte(OP::MULTIPLY); break;
        case TokenType::SLASH: emitByte(OP::DIVIDE); break;
        default: return; // unreachable
    }
}

uint8_t Compiler::makeConstant(double value) {
    int constant = currentChunk()->addConstant(value);
    if (constant > std::numeric_limits<uint8_t>::max()) {
        error("Too many constants in one chunk.");
        return 0;
    }
    return constant;
}

void Compiler::parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == nullptr) {
        error("Expect expression");
        return;
    }

    (this->*prefixRule)();

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        (this->*infixRule)();
    }
}

Compiler::ParseRule *Compiler::getRule(TokenType type) {
    return &rules[to_integral(type)];
}

