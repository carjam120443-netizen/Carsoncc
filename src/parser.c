#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void advance_token(Parser *p) { p->previous = p->current; p->current = lexer_next(&p->lexer); }
static int check(Parser *p, TokenKind k) { return p->current.kind == k; }
static int match(Parser *p, TokenKind k) { if (!check(p, k)) return 0; advance_token(p); return 1; }
static void error(Parser *p, const char *msg) { if (!p->had_error) fprintf(stderr, "CarsonCC: line %lu: %s (got %s)\n", (unsigned long)p->current.line, msg, token_kind_name(p->current.kind)); p->had_error = 1; }
static int consume(Parser *p, TokenKind k, const char *msg) { if (check(p, k)) { advance_token(p); return 1; } error(p, msg); return 0; }
static char *token_text(Token t) { char *s = malloc(t.length + 1); if (!s) return NULL; memcpy(s, t.start, t.length); s[t.length] = 0; return s; }
static AstNode *expression(Parser *p);

static AstNode *primary(Parser *p) {
    if (match(p, TOKEN_INTEGER)) { AstNode *n = ast_new(AST_INTEGER, p->previous.line); if (!n) return NULL; n->integer_value = strtol(p->previous.start, NULL, 10); return n; }
    if (match(p, TOKEN_IDENTIFIER)) { AstNode *n = ast_new(AST_VARIABLE, p->previous.line); if (!n) return NULL; n->name = token_text(p->previous); return n; }
    if (match(p, TOKEN_LPAREN)) { AstNode *n = expression(p); consume(p, TOKEN_RPAREN, "expected ')' after expression"); return n; }
    error(p, "expected expression"); return NULL;
}
static AstNode *factor(Parser *p) {
    AstNode *left = primary(p);
    while (check(p, TOKEN_STAR) || check(p, TOKEN_SLASH)) { Token op = p->current; advance_token(p); AstNode *right = primary(p); if (!left || !right) { ast_free(left); ast_free(right); return NULL; } AstNode *n = ast_new(AST_BINARY, op.line); if (!n) { ast_free(left); ast_free(right); return NULL; } n->left=left; n->right=right; n->op=(op.kind==TOKEN_STAR)?OP_MUL:OP_DIV; left=n; }
    return left;
}
static AstNode *expression(Parser *p) {
    AstNode *left = factor(p);
    while (check(p, TOKEN_PLUS) || check(p, TOKEN_MINUS)) { Token op=p->current; advance_token(p); AstNode *right=factor(p); if (!left||!right){ast_free(left);ast_free(right);return NULL;} AstNode*n=ast_new(AST_BINARY,op.line); if(!n){ast_free(left);ast_free(right);return NULL;} n->left=left;n->right=right;n->op=(op.kind==TOKEN_PLUS)?OP_ADD:OP_SUB;left=n; }
    return left;
}
static AstNode *statement(Parser *p) {
    if (match(p, TOKEN_LET)) {
        if (!check(p, TOKEN_IDENTIFIER)) { error(p, "expected variable name after 'let'"); return NULL; }
        Token name=p->current; advance_token(p); consume(p,TOKEN_EQUAL,"expected '=' after variable name"); AstNode*value=expression(p); consume(p,TOKEN_SEMICOLON,"expected ';' after declaration"); AstNode*n=ast_new(AST_LET,name.line); if(!n){ast_free(value);return NULL;} n->name=token_text(name);n->left=value;return n;
    }
    if (match(p, TOKEN_RETURN)) { AstNode*value=expression(p); consume(p,TOKEN_SEMICOLON,"expected ';' after return value"); AstNode*n=ast_new(AST_RETURN,p->previous.line); if(n)n->left=value;return n; }
    error(p,"expected statement"); return NULL;
}
static AstNode *function(Parser *p) {
    Token fn=p->previous; if(!check(p,TOKEN_IDENTIFIER)){error(p,"expected function name");return NULL;} Token name=p->current;advance_token(p); consume(p,TOKEN_LPAREN,"expected '(' after function name"); consume(p,TOKEN_RPAREN,"expected ')' after parameter list"); consume(p,TOKEN_LBRACE,"expected '{' before function body"); AstNode*f=ast_new(AST_FUNCTION,fn.line);if(!f)return NULL;f->name=token_text(name);
    while(!check(p,TOKEN_RBRACE)&&!check(p,TOKEN_EOF)){AstNode*s=statement(p);if(!s){ast_free(f);return NULL;}if(!ast_add_child(f,s)){ast_free(s);ast_free(f);return NULL;}}
    consume(p,TOKEN_RBRACE,"expected '}' after function body");return f;
}
void parser_init(Parser*p,const char*source){memset(p,0,sizeof(*p));lexer_init(&p->lexer,source);p->current=lexer_next(&p->lexer);}
AstNode*parser_parse(Parser*p){AstNode*program=ast_new(AST_PROGRAM,1);if(!program)return NULL;while(!check(p,TOKEN_EOF)){if(!match(p,TOKEN_FN)){error(p,"expected 'fn' at top level");ast_free(program);return NULL;}AstNode*f=function(p);if(!f||!ast_add_child(program,f)){ast_free(f);ast_free(program);return NULL;}}if(p->had_error){ast_free(program);return NULL;}return program;}
