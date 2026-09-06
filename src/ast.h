#ifndef CARSONCC_AST_H
#define CARSONCC_AST_H

/* Abstract Syntax Tree definitions will grow with the language grammar. */
typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_RETURN,
    AST_INTEGER,
    AST_BINARY
} AstKind;

typedef struct AstNode {
    AstKind kind;
    struct AstNode *left;
    struct AstNode *right;
    long integer_value;
} AstNode;

#endif
