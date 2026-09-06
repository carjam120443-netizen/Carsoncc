#include "ast.h"
#include <stdlib.h>

AstNode *ast_new(AstKind kind, size_t line) {
    AstNode *n = calloc(1, sizeof(*n));
    if (n) { n->kind = kind; n->line = line; }
    return n;
}

int ast_add_child(AstNode *parent, AstNode *child) {
    AstNode **p = realloc(parent->children, (parent->child_count + 1) * sizeof(*p));
    if (!p) return 0;
    parent->children = p;
    parent->children[parent->child_count++] = child;
    return 1;
}

void ast_free(AstNode *n) {
    if (!n) return;
    free(n->name);
    ast_free(n->left);
    ast_free(n->right);
    for (size_t i = 0; i < n->child_count; ++i) ast_free(n->children[i]);
    free(n->children);
    free(n);
}
