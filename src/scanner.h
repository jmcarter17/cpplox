#ifndef CPPLOX_SCANNER_H
#define CPPLOX_SCANNER_H


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


struct Token {
    TokenType type;
    const char* start;
    int length;
    int line;

    Token(TokenType type, const char* start, int length, int line);
};


class Scanner {
    const char* start;
    const char* current;
    int line;

public:
    Scanner(const char* source);

    bool isAtEnd();
    Token scanToken();
    Token makeToken(TokenType type);
    Token errorToken(const char* message) const;

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

//void initScanner(const char* source);



#endif //CPPLOX_SCANNER_H
