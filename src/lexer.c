#include "lexer.h"

#include <ctype.h>
#include <string.h>

static int at_end(const Lexer *lexer) {
    return lexer->position >= lexer->length;
}

static char peek(const Lexer *lexer) {
    return at_end(lexer) ? '\0' : lexer->source[lexer->position];
}

static char advance(Lexer *lexer) {
    return at_end(lexer) ? '\0' : lexer->source[lexer->position++];
}

static Token make_token(const Lexer *lexer, TokenKind kind, size_t start) {
    Token token = {
        kind,
        lexer->source + start,
        lexer->position - start,
        lexer->line
    };
    return token;
}

static Token error_token(const Lexer *lexer, size_t start) {
    return make_token(lexer, TOKEN_ERROR, start);
}

static TokenKind keyword(const char *text, size_t length) {
    if (length == 2 && !strncmp(text, "fn", 2)) return TOKEN_FN;
    if (length == 3 && !strncmp(text, "let", 3)) return TOKEN_LET;
    if (length == 6 && !strncmp(text, "return", 6)) return TOKEN_RETURN;
    if (length == 3 && !strncmp(text, "int", 3)) return TOKEN_INT;
    return TOKEN_IDENTIFIER;
}

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
}

Token lexer_next(Lexer *lexer) {
    while (!at_end(lexer)) {
        char character = peek(lexer);

        if (character == ' ' || character == '\t' || character == '\r') {
            advance(lexer);
            continue;
        }

        if (character == '\n') {
            advance(lexer);
            lexer->line++;
            continue;
        }

        if (character == '/' && lexer->position + 1 < lexer->length &&
            lexer->source[lexer->position + 1] == '/') {
            while (!at_end(lexer) && peek(lexer) != '\n') {
                advance(lexer);
            }
            continue;
        }

        break;
    }

    size_t start = lexer->position;
    if (at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF, start);
    }

    char character = advance(lexer);

    if (isalpha((unsigned char)character) || character == '_') {
        while (isalnum((unsigned char)peek(lexer)) || peek(lexer) == '_') {
            advance(lexer);
        }
        return make_token(lexer, keyword(lexer->source + start,
                                         lexer->position - start), start);
    }

    if (isdigit((unsigned char)character)) {
        while (isdigit((unsigned char)peek(lexer))) {
            advance(lexer);
        }
        return make_token(lexer, TOKEN_INTEGER, start);
    }

    if (character == '"') {
        while (!at_end(lexer) && peek(lexer) != '"') {
            if (peek(lexer) == '\n') {
                lexer->line++;
            }

            if (peek(lexer) == '\\') {
                advance(lexer);
                if (!at_end(lexer)) advance(lexer);
            } else {
                advance(lexer);
            }
        }

        if (at_end(lexer)) {
            return error_token(lexer, start);
        }

        advance(lexer);
        return make_token(lexer, TOKEN_STRING, start);
    }

    switch (character) {
        case '+': return make_token(lexer, TOKEN_PLUS, start);
        case '-': return make_token(lexer, TOKEN_MINUS, start);
        case '*': return make_token(lexer, TOKEN_STAR, start);
        case '/': return make_token(lexer, TOKEN_SLASH, start);
        case '=': return make_token(lexer, TOKEN_EQUAL, start);
        case ';': return make_token(lexer, TOKEN_SEMICOLON, start);
        case ',': return make_token(lexer, TOKEN_COMMA, start);
        case '(': return make_token(lexer, TOKEN_LPAREN, start);
        case ')': return make_token(lexer, TOKEN_RPAREN, start);
        case '{': return make_token(lexer, TOKEN_LBRACE, start);
        case '}': return make_token(lexer, TOKEN_RBRACE, start);
        default: return error_token(lexer, start);
    }
}

const char *token_kind_name(TokenKind kind) {
    static const char *names[] = {
        "eof", "error", "identifier", "integer", "string",
        "fn", "let", "return", "int", "+", "-", "*", "/",
        "=", ";", ",", "(", ")", "{", "}"
    };

    return kind <= TOKEN_RBRACE ? names[kind] : "unknown";
}
