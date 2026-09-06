#ifndef CARSONCC_LEXER_H
#define CARSONCC_LEXER_H

/* Token kinds used by the Carson language lexer. */
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_STRING,
    TOKEN_FN,
    TOKEN_LET,
    TOKEN_RETURN,
    TOKEN_INT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_SEMICOLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE
} TokenKind;

typedef struct {
    TokenKind kind;
    const char *start;
    unsigned long length;
    unsigned long line;
} Token;

#endif
