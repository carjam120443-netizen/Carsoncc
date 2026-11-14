#include "ast.h"

#include <stdlib.h>
#include <string.h>

static char *dup_text(const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = malloc(length);

    if (copy) {
        memcpy(copy, text, length);
    }

    return copy;
}

AstNode *ast_new(AstKind kind, size_t line) {
    AstNode *node = calloc(1, sizeof(*node));

    if (node) {
        node->kind = kind;
        node->line = line;
    }

    return node;
}

int ast_add_child(AstNode *parent, AstNode *child) {
    AstNode **children = realloc(
        parent->children,
        (parent->child_count + 1) * sizeof(*children)
    );

    if (!children) {
        return 0;
    }

    parent->children = children;
    parent->children[parent->child_count++] = child;
    return 1;
}

int ast_add_parameter(AstNode *function, const char *name) {
    size_t index = function->parameter_count;
    char *text = dup_text(name);

    if (!text) {
        return 0;
    }

    char **parameters = realloc(
        function->parameters,
        (index + 1) * sizeof(*parameters)
    );

    if (!parameters) {
        free(text);
        return 0;
    }

    parameters[index] = text;
    function->parameters = parameters;
    function->parameter_count = index + 1;
    return 1;
}

void ast_free(AstNode *node) {
    if (!node) {
        return;
    }

    free(node->name);

    for (size_t i = 0; i < node->parameter_count; ++i) {
        free(node->parameters[i]);
    }

    free(node->parameters);
    ast_free(node->left);
    ast_free(node->right);

    for (size_t i = 0; i < node->child_count; ++i) {
        ast_free(node->children[i]);
    }

    free(node->children);
    free(node);
}
