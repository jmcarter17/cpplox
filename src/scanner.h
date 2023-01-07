#ifndef CPPLOX_SCANNER_H
#define CPPLOX_SCANNER_H

#include <fmt/format.h>
#include <magic_enum.hpp>


enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS,
    SEMICOLON, SLASH, STAR,
    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    // Literals.
    IDENTIFIER, STRING, NUMBER,
    // Keywords.
    AND, CLASS, ELSE, FALSE,
    FOR, FUN, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS,
    TRUE, VAR, WHILE,

    ERROR, EOFILE
};

template<>
struct fmt::formatter<TokenType> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext &ctx) {
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(TokenType const &type, FormatContext &ctx) {
        return fmt::format_to(ctx.out(), "TokenType::{}", magic_enum::enum_name(type));
    }
};


struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;

    Token(TokenType type, std::string_view lexeme, int line);
};


class Scanner {
    const char* start;
    const char* current;
    int line;

public:
    explicit Scanner(std::string_view source);

    bool isAtEnd();
    Token scanToken();
    Token makeToken(TokenType type);
    Token errorToken(std::string_view message) const;

    char advance();
    bool match(char i);
    void skipWhitespace();
    char peek();
    char peekNext();
    Token string();
    static bool isDigit(char c);
    Token number();
    static bool isAlpha(char c);
    Token identifier();
    TokenType identifierType();
    TokenType checkKeyword(int begin, int length, const char *rest, TokenType type);
};

#endif //CPPLOX_SCANNER_H
