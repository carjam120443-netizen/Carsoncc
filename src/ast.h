#ifndef CARSONCC_AST_H
#define CARSONCC_AST_H

#include <stddef.h>

typedef enum { AST_PROGRAM, AST_FUNCTION, AST_BLOCK, AST_RETURN, AST_LET, AST_VARIABLE, AST_INTEGER, AST_BINARY } AstKind;
typedef enum { OP_ADD, OP_SUB, OP_MUL, OP_DIV } BinaryOp;

typedef struct AstNode AstNode;
struct AstNode {
    AstKind kind;
    size_t line;
    char *name;
    long integer_value;
    BinaryOp op;
    AstNode *left;
    AstNode *right;
    AstNode **children;
    size_t child_count;
};

AstNode *ast_new(AstKind kind, size_t line);
void ast_free(AstNode *node);
int ast_add_child(AstNode *parent, AstNode *child);

#endif
