#include "lexer.h"
#include <ctype.h>
#include <string.h>

static int at_end(const Lexer *l) { return l->position >= l->length; }
static char peek(const Lexer *l) { return at_end(l) ? '\0' : l->source[l->position]; }
static char advance(Lexer *l) { return at_end(l) ? '\0' : l->source[l->position++]; }

static Token make_token(const Lexer *l, TokenKind kind, size_t start) {
    Token t = { kind, l->source + start, l->position - start, l->line };
    return t;
}

static Token error_token(const Lexer *l, size_t start) {
    return make_token(l, TOKEN_ERROR, start);
}

static TokenKind keyword(const char *s, size_t n) {
    if (n == 2 && strncmp(s, "fn", 2) == 0) return TOKEN_FN;
    if (n == 3 && strncmp(s, "let", 3) == 0) return TOKEN_LET;
    if (n == 6 && strncmp(s, "return", 6) == 0) return TOKEN_RETURN;
    if (n == 3 && strncmp(s, "int", 3) == 0) return TOKEN_INT;
    return TOKEN_IDENTIFIER;
}

void lexer_init(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->length = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
}

Token lexer_next(Lexer *l) {
    while (!at_end(l)) {
        char c = peek(l);
        if (c == ' ' || c == '\t' || c == '\r') { advance(l); continue; }
        if (c == '\n') { advance(l); l->line++; continue; }
        if (c == '/' && l->position + 1 < l->length && l->source[l->position + 1] == '/') {
            while (!at_end(l) && peek(l) != '\n') advance(l);
            continue;
        }
        break;
    }

    size_t start = l->position;
    if (at_end(l)) return make_token(l, TOKEN_EOF, start);
    char c = advance(l);

    if (isalpha((unsigned char)c) || c == '_') {
        while (isalnum((unsigned char)peek(l)) || peek(l) == '_') advance(l);
        return make_token(l, keyword(l->source + start, l->position - start), start);
    }
    if (isdigit((unsigned char)c)) {
        while (isdigit((unsigned char)peek(l))) advance(l);
        return make_token(l, TOKEN_INTEGER, start);
    }
    if (c == '"') {
        while (!at_end(l) && peek(l) != '"') {
            if (peek(l) == '\n') l->line++;
            if (peek(l) == '\\') { advance(l); if (!at_end(l)) advance(l); }
            else advance(l);
        }
        if (at_end(l)) return error_token(l, start);
        advance(l);
        return make_token(l, TOKEN_STRING, start);
    }

    switch (c) {
        case '+': return make_token(l, TOKEN_PLUS, start);
        case '-': return make_token(l, TOKEN_MINUS, start);
        case '*': return make_token(l, TOKEN_STAR, start);
        case '/': return make_token(l, TOKEN_SLASH, start);
        case '=': return make_token(l, TOKEN_EQUAL, start);
        case ';': return make_token(l, TOKEN_SEMICOLON, start);
        case '(': return make_token(l, TOKEN_LPAREN, start);
        case ')': return make_token(l, TOKEN_RPAREN, start);
        case '{': return make_token(l, TOKEN_LBRACE, start);
        case '}': return make_token(l, TOKEN_RBRACE, start);
        default: return error_token(l, start);
    }
}

const char *token_kind_name(TokenKind kind) {
    static const char *names[] = { "eof", "error", "identifier", "integer", "string", "fn", "let", "return", "int", "+", "-", "*", "/", "=", ";", "(", ")", "{", "}" };
    return kind <= TOKEN_RBRACE ? names[kind] : "unknown";
}
