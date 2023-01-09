#include <cstring>
#include "scanner.h"


Token::Token(TokenType type, std::string_view lexeme, int line) : type{type}, lexeme{lexeme}, line{line} {}

Token::Token() : type{TokenType::ERROR}, lexeme{}, line{} {}

Scanner::Scanner(std::string_view source) {
    start = source.data();
    current = source.data();
    line = 1;
}

bool Scanner::isAtEnd() {
    return *current == '\0';
}

Token Scanner::scanToken() {
    skipWhitespace();
    start = current;

    if (isAtEnd()) return makeToken(TokenType::EOFILE);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(':
            return makeToken(TokenType::LEFT_PAREN);
        case ')':
            return makeToken(TokenType::RIGHT_PAREN);
        case '{':
            return makeToken(TokenType::LEFT_BRACE);
        case '}':
            return makeToken(TokenType::RIGHT_BRACE);
        case ';':
            return makeToken(TokenType::SEMICOLON);
        case ',':
            return makeToken(TokenType::COMMA);
        case '.':
            return makeToken(TokenType::DOT);
        case '-':
            return makeToken(TokenType::MINUS);
        case '+':
            return makeToken(TokenType::PLUS);
        case '/':
            return makeToken(TokenType::SLASH);
        case '*':
            return makeToken(TokenType::STAR);
        case '!':
            return makeToken(
                    match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
        case '=':
            return makeToken(
                    match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
        case '<':
            return makeToken(
                    match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
        case '>':
            return makeToken(
                    match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
        case '"':
            return string();
    }

    return errorToken("Unexpected character");
}

auto Scanner::makeToken(TokenType type) -> Token {
    return {type, {start, current}, line};
}

auto Scanner::errorToken(std::string_view message) const -> Token {
    return {TokenType::ERROR, message, line};
}

char Scanner::advance() {
    return *current++;
}

char Scanner::peek() {
    return *current;
}

bool Scanner::match(char expected) {
    if (isAtEnd()) return false;
    if (*current != expected) return false;

    current++;
    return true;
}

void Scanner::skipWhitespace() {
    while (1) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    while (peek() != '\n' && !(isAtEnd())) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

char Scanner::peekNext() {
    if (isAtEnd()) return '\0';
    return current[1];
}

Token Scanner::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    advance();
    return makeToken(TokenType::STRING);
}

bool Scanner::isDigit(char c) {
    return c >= '0' && c <= '9';
}

Token Scanner::number() {
    while (isDigit(peek())) advance();

    if (peek() == '.' && isDigit(peekNext())) {
        advance();
        while (isDigit(peek())) advance();
    }
    return makeToken(TokenType::NUMBER);
}

bool Scanner::isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

Token Scanner::identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();
    return makeToken(identifierType());
}

TokenType Scanner::identifierType() {
    switch (start[0]) {
        case 'a':
            return checkKeyword(1, 2, "nd", TokenType::AND);
        case 'c':
            return checkKeyword(1, 4, "lass", TokenType::CLASS);
        case 'e':
            return checkKeyword(1, 3, "lse", TokenType::ELSE);
        case 'i':
            return checkKeyword(1, 1, "f", TokenType::IF);
        case 'n':
            return checkKeyword(1, 2, "il", TokenType::NIL);
        case 'o':
            return checkKeyword(1, 1, "r", TokenType::OR);
        case 'p':
            return checkKeyword(1, 4, "rint", TokenType::PRINT);
        case 'r':
            return checkKeyword(1, 5, "eturn", TokenType::RETURN);
        case 's':
            return checkKeyword(1, 4, "uper", TokenType::SUPER);
        case 'v':
            return checkKeyword(1, 2, "ar", TokenType::VAR);
        case 'w':
            return checkKeyword(1, 4, "hile", TokenType::WHILE);
        case 'f':
            if ((current - start) > 1) {
                switch (start[1]) {
                    case 'a':
                        return checkKeyword(2, 3, "lse", TokenType::FALSE);
                    case 'o':
                        return checkKeyword(2, 1, "r", TokenType::FOR);
                    case 'u':
                        return checkKeyword(2, 1, "n", TokenType::FUN);
                }
            }
            break;
        case 't':
            if ((current - start) > 1) {
                switch (start[1]) {
                    case 'h':
                        return checkKeyword(2, 2, "is", TokenType::THIS);
                    case 'r':
                        return checkKeyword(2, 2, "ue", TokenType::TRUE);
                }
            }
            break;
    }
    return TokenType::IDENTIFIER;
}

TokenType Scanner::checkKeyword(int begin, int length, const char *rest, TokenType type) {
    if (current - start == begin + length && memcmp(start + begin, rest, length) == 0) {
        return type;
    }
    return TokenType::IDENTIFIER;
}
