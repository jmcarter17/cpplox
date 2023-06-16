#include "compiler.h"
#include "scanner.h"
#include <fmt/core.h>
#include <cstdint>
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
        {TokenType::IDENTIFIER,    {&Compiler::variable, nullptr,           Precedence::NONE}},
        {TokenType::STRING,        {&Compiler::string,   nullptr,           Precedence::NONE}},
        {TokenType::NUMBER,        {&Compiler::number,   nullptr,           Precedence::NONE}},
        {TokenType::AND,           {nullptr,             &Compiler::and_,   Precedence::AND}},
        {TokenType::CLASS,         {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::ELSE,          {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::FALSE,         {&Compiler::literal,  nullptr,           Precedence::NONE}},
        {TokenType::FOR,           {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::FUN,           {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::IF,            {nullptr,             nullptr,           Precedence::NONE}},
        {TokenType::NIL,           {&Compiler::literal,  nullptr,           Precedence::NONE}},
        {TokenType::OR,            {nullptr,             &Compiler::or_,    Precedence::OR}},
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

    while (!match(TokenType::EOFILE)) {
        declaration();
    }

//    consume(TokenType::EOFILE, "Expect end of expression");
//
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

bool Compiler::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool Compiler::check(TokenType type) const {
    return parser.current.type == type;
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

int Compiler::emitJump(OP op) {
    emitByte(op);
    emitByte(0xff);
    emitByte(0xff);
    return static_cast<int>(compilingChunk->code.size()) - 2;
}

void Compiler::emitLoop(unsigned long start) {
    emitByte(OP::LOOP);

    auto offset = compilingChunk->code.size() - start + 2;
    if (offset > std::numeric_limits<uint16_t>::max()) error("Loop body too large");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::endCompiler() {
    emitReturn();
    if constexpr (DEBUG_PRINT_CODE) {
        if (!parser.hadError) {
            Disassembler::disassembleChunk(*currentChunk(), "Code");
        }
    }
}

void Compiler::beginScope() {
    current.scopeDepth++;
}

void Compiler::endScope() {
    current.scopeDepth--;

    while (current.localCount > 0 && current.locals[current.localCount-1].depth > current.scopeDepth){
        emitByte(OP::POP);
        current.localCount--;
    }
}

void Compiler::declaration() {
    if (match(TokenType::VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

void Compiler::varDeclaration() {
    uint8_t global = parseVariable("Expect variable name");

    if (match(TokenType::EQUAL)) {
        expression();
    } else {
        emitByte(OP::NIL);
    }
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration");

    defineVariable(global);
}

void Compiler::statement() {
    if (match(TokenType::PRINT)) {
        printStatement();
    } else if (match(TokenType::FOR)) {
        forStatement();
    } else if (match(TokenType::IF)) {
        ifStatement();
    } else if (match(TokenType::WHILE)) {
        whileStatement();
    } else if (match(TokenType::LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

void Compiler::printStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    emitByte(OP::PRINT);
}

void Compiler::expressionStatement() {
    expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression");
    emitByte(OP::POP);
}

void Compiler::synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TokenType::EOFILE) {
        if (parser.previous.type == TokenType::SEMICOLON) return;
        switch (parser.current.type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;

            default:
                ; // do nothin
        }
        advance();
    }
}

void Compiler::expression() {
    parsePrecedence(Precedence::ASSIGNMENT);
}

void Compiler::block() {
    while(!check(TokenType::RIGHT_BRACE) && !check(TokenType::EOFILE)) {
        declaration();
    }
    consume(TokenType::RIGHT_BRACE, "Expect, '}' after block.");
}

void Compiler::declareVariable() {
    if (current.scopeDepth == 0) {
        return;
    }

    Token name = parser.previous;
    for (int i = current.localCount - 1; i >= 0 ; --i) {
        auto& local = current.locals[i];
        if (local.depth != -1 && local.depth < current.scopeDepth) {
            break;
        }

        if (identifiersEqual(name, local.name)) {
            error("Already a variable with this name in this scope.");
        }
    }

    addLocal(name);
}

void Compiler::addLocal(Token name) {
    if (current.localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }

    Local& local = current.locals[current.localCount++];
    local.name = name;
    local.depth = -1;
}

void Compiler::number(bool canAssign) {
    auto lexeme = parser.previous.lexeme;
    auto value = std::strtod(lexeme.data(), nullptr);
    emitConstant(number_val(value));
}

void Compiler::string(bool canAssign) {
    auto lexeme = parser.previous.lexeme;
    auto str = std::string_view(lexeme.data() + 1, lexeme.size() - 2); // remove opening and trailing "
    emitConstant(obj_val(copyString(str)));
}

void Compiler::variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

void Compiler::namedVariable(Token name, bool canAssign) {
    OP getOp, setOp;
    int arg = resolveLocal(name);
    if (arg != -1) {
        getOp = OP::GET_LOCAL;
        setOp = OP::SET_LOCAL;
    } else {
        arg = identifierConstant(name);
        getOp = OP::GET_GLOBAL;
        setOp = OP::SET_GLOBAL;
    }

    if (canAssign && match(TokenType::EQUAL)) {
        expression();
        emitBytes(setOp, arg);
    } else {
        emitBytes(getOp, arg);
    }
}

void Compiler::grouping(bool canAssign) {
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

void Compiler::unary(bool canAssign) {
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

void Compiler::binary(bool canAssign) {
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

void Compiler::literal(bool canAssign) {
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

void Compiler::and_(bool canAssign){
    int endJump = emitJump(OP::JUMP_IF_FALSE);

    emitByte(OP::POP);
    parsePrecedence(Precedence::AND);

    patchJump(endJump);
}

void Compiler::or_(bool canAssign) {
    int endJump = emitJump(OP::JUMP_IF_TRUE);

    emitByte(OP::POP);
    parsePrecedence(Precedence::OR);
    patchJump(endJump);
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

    bool canAssign = precedence <= Precedence::ASSIGNMENT;
    (this->*prefixRule)(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        (this->*infixRule)(canAssign);
    }
}

uint8_t Compiler::identifierConstant(const Token& token) {
    return makeConstant(obj_val(copyString(token.lexeme)));
}

bool Compiler::identifiersEqual(const Token &a, const Token &b) {
    return a.lexeme == b.lexeme;
}

int Compiler::resolveLocal(Token &name){
    for (int i = current.localCount - 1; i >= 0; --i) {
        auto &local = current.locals[i];
        if (identifiersEqual(name, local.name)) {
            if (local.depth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

uint8_t Compiler::parseVariable(std::string_view message) {
    consume(TokenType::IDENTIFIER, message);

    declareVariable();
    if (current.scopeDepth > 0) return 0;

    return identifierConstant(parser.previous);
}

Compiler::ParseRule *Compiler::getRule(TokenType type) {
    return &rules[type];
}

ObjString* Compiler::copyString(std::string_view value) {
    auto [it, val] = vm->strings.emplace(value);
    auto view = std::string_view(*it);
    auto obj = new ObjString(ObjType::STRING, vm->objects, view);
    vm->objects = obj;
    return obj;
}

void Compiler::defineVariable(uint8_t global) {
    if (current.scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitBytes(OP::DEFINE_GLOBAL, global);
}

void Compiler::markInitialized() {
    current.locals[current.localCount - 1].depth = current.scopeDepth;
}

void Compiler::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    int thenJump = emitJump(OP::JUMP_IF_FALSE);
    emitByte(OP::POP);
    statement();
    int elseJump = emitJump(OP::JUMP);
    patchJump(thenJump);
    emitByte(OP::POP);

    if (match(TokenType::ELSE)) statement();
    patchJump(elseJump);
}

void Compiler::forStatement() {
    beginScope();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(TokenType::SEMICOLON)) {
        // Non initializer.
    } else if (match(TokenType::VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    auto loopStart = compilingChunk->code.size();
    int exitJump = -1;
    if (!match(TokenType::SEMICOLON)) {
        expression();
        consume(TokenType::SEMICOLON, "Expect ';' after loop conditions.");

        exitJump = emitJump(OP::JUMP_IF_FALSE);
        emitByte(OP::POP);
    }

    if (!match(TokenType::RIGHT_PAREN)) {
        int bodyJump = emitJump(OP::JUMP);
        auto incrementStart = compilingChunk->code.size();
        expression();
        emitByte(OP::POP);
        consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP::POP);
    }
    endScope();
}

void Compiler::whileStatement() {
    auto loopStart = compilingChunk->code.size();
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP::JUMP_IF_FALSE);
    emitByte(OP::POP);
    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP::POP);
}

void Compiler::patchJump(int offset) {
    auto jumpsize = compilingChunk->code.size() - offset - 2;

    if (jumpsize > std::numeric_limits<uint16_t>::max()) {
        error("Too much code to jump over.");
    }

    compilingChunk->code[offset] = (jumpsize >> 8) & 0xff;
    compilingChunk->code[offset + 1] = jumpsize & 0xff;
}

