#include "compiler.h"
#include "scanner.h"
#include <fmt/core.h>
#include "common.h"
#include "Disassembler.h"

Compiler::Compiler(std::string_view source, VM* vm) :
        scanner{source}, parser{}, vm{vm}, rules{
        {TokenType::LEFT_PAREN,    {&Compiler::grouping, nullptr,           Precedence::NONE}},
        {TokenType::RIGHT_PAREN,   {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::LEFT_BRACE,    {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::RIGHT_BRACE,   {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::COMMA,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::DOT,           {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::MINUS,         {&Compiler::unary,    &Compiler::binary, Precedence::TERM}},
        {TokenType::PLUS,          {nullptr,             &Compiler::binary, Precedence::TERM}},
        {TokenType::SEMICOLON,     {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::SLASH,         {nullptr,             &Compiler::binary, Precedence::FACTOR}},
        {TokenType::STAR,          {nullptr,             &Compiler::binary, Precedence::FACTOR}},
        {TokenType::BANG,          {&Compiler::unary,    nullptr,           Precedence::NONE}},
        {TokenType::BANG_EQUAL,    {nullptr,             &Compiler::binary, Precedence::EQUALITY}},
        {TokenType::EQUAL,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::EQUAL_EQUAL,   {nullptr,             &Compiler::binary, Precedence::EQUALITY}},
        {TokenType::GREATER,       {nullptr,             &Compiler::binary, Precedence::COMPARISON}},
        {TokenType::GREATER_EQUAL, {nullptr,             &Compiler::binary, Precedence::COMPARISON}},
        {TokenType::LESS,          {nullptr,             &Compiler::binary, Precedence::COMPARISON}},
        {TokenType::LESS_EQUAL,    {nullptr,             &Compiler::binary, Precedence::COMPARISON}},
        {TokenType::IDENTIFIER,    {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::STRING,        {&Compiler::string,   nullptr,           Precedence::NONE}},
        {TokenType::NUMBER,        {&Compiler::number,   nullptr,           Precedence::NONE}},
        {TokenType::AND,           {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::CLASS,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::ELSE,          {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::FALSE,         {&Compiler::literal,  nullptr,           Precedence::NONE}},
        {TokenType::FOR,           {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::FUN,           {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::IF,            {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::NIL,           {&Compiler::literal,  nullptr,           Precedence::NONE}},
        {TokenType::OR,            {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::PRINT,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::RETURN,        {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::SUPER,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::THIS,          {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::TRUE,          {&Compiler::literal,  nullptr,           Precedence::NONE}},
        {TokenType::VAR,           {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::WHILE,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::ERROR,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::EOFILE,        {nullptr,             nullptr,           Precedence::NONE}},
}, compilingChunk{} {}

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

void Compiler::emitConstant(Value value) {
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
    emitConstant(number_val(value));
}

void Compiler::string() {
    auto lexeme = parser.previous.lexeme;
    emitConstant(obj_val(copyString(lexeme)));
}

void Compiler::grouping() {
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary() {
    TokenType operatorType = parser.previous.type;

    // compile the operand
    parsePrecedence(Precedence::UNARY);

    switch (operatorType) {
        case TokenType::MINUS:
            emitByte(OP::NEGATE);
            break;
        case TokenType::BANG:
            emitByte(OP::NOT);
            break;
        default:
            return; // unreachable
    }
}

void Compiler::binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(to_integral(rule->precedence) + 1));

    switch (operatorType) {
        case TokenType::BANG_EQUAL:
            emitByte(OP::NOT_EQUAL);
            break;
        case TokenType::EQUAL_EQUAL:
            emitByte(OP::EQUAL);
            break;
        case TokenType::GREATER:
            emitByte(OP::GREATER);
            break;
        case TokenType::GREATER_EQUAL:
            emitByte(OP::GREATER_EQUAL);
            break;
        case TokenType::LESS:
            emitByte(OP::LESS);
            break;
        case TokenType::LESS_EQUAL:
            emitByte(OP::LESS_EQUAL);
            break;
        case TokenType::PLUS:
            emitByte(OP::ADD);
            break;
        case TokenType::MINUS:
            emitByte(OP::SUBTRACT);
            break;
        case TokenType::STAR:
            emitByte(OP::MULTIPLY);
            break;
        case TokenType::SLASH:
            emitByte(OP::DIVIDE);
            break;
        default:
            return; // unreachable
    }
}

void Compiler::literal() {
    switch (parser.previous.type) {
        case TokenType::FALSE:
            emitByte(OP::FALSE);
            break;
        case TokenType::TRUE:
            emitByte(OP::TRUE);
            break;
        case TokenType::NIL:
            emitByte(OP::NIL);
            break;
        default:
            return; // unreacheable.
    }
}

uint8_t Compiler::makeConstant(Value value) {
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
    return &rules[type];
}

ObjString* Compiler::copyString(std::string_view lexeme) {
//    auto view = std::string_view(lexeme.data() + 1, lexeme.size() - 2);
    auto [it, val] = vm->strings.emplace(std::string_view(lexeme.data() + 1, lexeme.size() - 2));
    auto view = std::string_view(*it);
    auto obj = new ObjString(ObjType::STRING, vm->objects, view);
    vm->objects = obj;
    return obj;
}

