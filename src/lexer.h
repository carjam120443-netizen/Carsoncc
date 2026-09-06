#ifndef CARSONCC_LEXER_H
#define CARSONCC_LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_EOF = 0, TOKEN_ERROR, TOKEN_IDENTIFIER, TOKEN_INTEGER, TOKEN_STRING,
    TOKEN_FN, TOKEN_LET, TOKEN_RETURN, TOKEN_INT,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH, TOKEN_EQUAL,
    TOKEN_SEMICOLON, TOKEN_COMMA, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE
} TokenKind;

typedef struct { TokenKind kind; const char *start; size_t length; size_t line; } Token;
typedef struct { const char *source; size_t length; size_t position; size_t line; } Lexer;

void lexer_init(Lexer *lexer, const char *source);
Token lexer_next(Lexer *lexer);
const char *token_kind_name(TokenKind kind);

#endif
