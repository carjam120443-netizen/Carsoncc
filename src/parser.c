#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void advance_token(Parser *parser) {
    parser->previous = parser->current;
    parser->current = lexer_next(&parser->lexer);
}

static int check(Parser *parser, TokenKind kind) {
    return parser->current.kind == kind;
}

static int match(Parser *parser, TokenKind kind) {
    if (!check(parser, kind)) {
        return 0;
    }

    advance_token(parser);
    return 1;
}

static void error(Parser *parser, const char *message) {
    if (!parser->had_error) {
        fprintf(stderr, "CarsonCC: line %lu: %s (got %s)\n",
                (unsigned long)parser->current.line,
                message,
                token_kind_name(parser->current.kind));
    }

    parser->had_error = 1;
}

static int consume(Parser *parser, TokenKind kind, const char *message) {
    if (check(parser, kind)) {
        advance_token(parser);
        return 1;
    }

    error(parser, message);
    return 0;
}

static char *token_text(Token token) {
    char *text = malloc(token.length + 1);

    if (!text) {
        return NULL;
    }

    memcpy(text, token.start, token.length);
    text[token.length] = '\0';
    return text;
}

static AstNode *expression(Parser *parser);

static AstNode *primary(Parser *parser) {
    if (match(parser, TOKEN_INTEGER)) {
        AstNode *node = ast_new(AST_INTEGER, parser->previous.line);

        if (node) {
            node->integer_value = strtol(parser->previous.start, NULL, 10);
        }

        return node;
    }

    if (match(parser, TOKEN_IDENTIFIER)) {
        Token name = parser->previous;

        if (match(parser, TOKEN_LPAREN)) {
            AstNode *call = ast_new(AST_CALL, name.line);

            if (!call) {
                return NULL;
            }

            call->name = token_text(name);

            if (!check(parser, TOKEN_RPAREN)) {
                do {
                    AstNode *argument = expression(parser);

                    if (!argument || !ast_add_child(call, argument)) {
                        ast_free(argument);
                        ast_free(call);
                        return NULL;
                    }
                } while (match(parser, TOKEN_COMMA));
            }

            consume(parser, TOKEN_RPAREN, "expected ')' after arguments");
            return call;
        }

        AstNode *variable = ast_new(AST_VARIABLE, name.line);

        if (variable) {
            variable->name = token_text(name);
        }

        return variable;
    }

    if (match(parser, TOKEN_LPAREN)) {
        AstNode *node = expression(parser);
        consume(parser, TOKEN_RPAREN, "expected ')' after expression");
        return node;
    }

    error(parser, "expected expression");
    return NULL;
}

static AstNode *factor(Parser *parser) {
    AstNode *left = primary(parser);

    while (check(parser, TOKEN_STAR) || check(parser, TOKEN_SLASH)) {
        Token operator = parser->current;
        advance_token(parser);

        AstNode *right = primary(parser);

        if (!left || !right) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        AstNode *node = ast_new(AST_BINARY, operator.line);

        if (!node) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        node->left = left;
        node->right = right;
        node->op = operator.kind == TOKEN_STAR ? OP_MUL : OP_DIV;
        left = node;
    }

    return left;
}

static AstNode *expression(Parser *parser) {
    AstNode *left = factor(parser);

    while (check(parser, TOKEN_PLUS) || check(parser, TOKEN_MINUS)) {
        Token operator = parser->current;
        advance_token(parser);

        AstNode *right = factor(parser);

        if (!left || !right) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        AstNode *node = ast_new(AST_BINARY, operator.line);

        if (!node) {
            ast_free(left);
            ast_free(right);
            return NULL;
        }

        node->left = left;
        node->right = right;
        node->op = operator.kind == TOKEN_PLUS ? OP_ADD : OP_SUB;
        left = node;
    }

    return left;
}

static AstNode *statement(Parser *parser) {
    if (match(parser, TOKEN_LET)) {
        if (!check(parser, TOKEN_IDENTIFIER)) {
            error(parser, "expected variable name after 'let'");
            return NULL;
        }

        Token name = parser->current;
        advance_token(parser);
        consume(parser, TOKEN_EQUAL, "expected '=' after variable name");

        AstNode *value = expression(parser);
        consume(parser, TOKEN_SEMICOLON, "expected ';' after declaration");

        AstNode *node = ast_new(AST_LET, name.line);

        if (!node) {
            ast_free(value);
            return NULL;
        }

        node->name = token_text(name);
        node->left = value;
        return node;
    }

    if (match(parser, TOKEN_RETURN)) {
        AstNode *value = expression(parser);
        consume(parser, TOKEN_SEMICOLON, "expected ';' after return value");

        AstNode *node = ast_new(AST_RETURN, parser->previous.line);

        if (node) {
            node->left = value;
        }

        return node;
    }

    error(parser, "expected statement");
    return NULL;
}

static AstNode *function(Parser *parser) {
    Token function_token = parser->previous;

    if (!check(parser, TOKEN_IDENTIFIER)) {
        error(parser, "expected function name");
        return NULL;
    }

    Token name = parser->current;
    advance_token(parser);
    consume(parser, TOKEN_LPAREN, "expected '(' after function name");

    AstNode *function_node = ast_new(AST_FUNCTION, function_token.line);

    if (!function_node) {
        return NULL;
    }

    function_node->name = token_text(name);

    if (!check(parser, TOKEN_RPAREN)) {
        do {
            if (!check(parser, TOKEN_IDENTIFIER)) {
                error(parser, "expected parameter name");
                ast_free(function_node);
                return NULL;
            }

            Token parameter = parser->current;
            advance_token(parser);

            char *text = token_text(parameter);

            if (!text || !ast_add_parameter(function_node, text)) {
                free(text);
                ast_free(function_node);
                return NULL;
            }

            free(text);
        } while (match(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RPAREN, "expected ')' after parameter list");
    consume(parser, TOKEN_LBRACE, "expected '{' before function body");

    while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
        AstNode *child = statement(parser);

        if (!child) {
            ast_free(function_node);
            return NULL;
        }

        if (!ast_add_child(function_node, child)) {
            ast_free(child);
            ast_free(function_node);
            return NULL;
        }
    }

    consume(parser, TOKEN_RBRACE, "expected '}' after function body");
    return function_node;
}

void parser_init(Parser *parser, const char *source) {
    memset(parser, 0, sizeof(*parser));
    lexer_init(&parser->lexer, source);
    parser->current = lexer_next(&parser->lexer);
}

AstNode *parser_parse(Parser *parser) {
    AstNode *program = ast_new(AST_PROGRAM, 1);

    if (!program) {
        return NULL;
    }

    while (!check(parser, TOKEN_EOF)) {
        if (!match(parser, TOKEN_FN)) {
            error(parser, "expected 'fn' at top level");
            ast_free(program);
            return NULL;
        }

        AstNode *function_node = function(parser);

        if (!function_node || !ast_add_child(program, function_node)) {
            ast_free(function_node);
            ast_free(program);
            return NULL;
        }
    }

    if (parser->had_error) {
        ast_free(program);
        return NULL;
    }

    return program;
}
