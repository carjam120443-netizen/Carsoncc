#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const char *name; size_t arity; } FunctionSymbol;

static int has_name(char **names, size_t count, const char *name) {
    for (size_t i = 0; i < count; ++i) if (strcmp(names[i], name) == 0) return 1;
    return 0;
}

static int find_function(const FunctionSymbol *functions, size_t count, const char *name, size_t *arity) {
    for (size_t i = 0; i < count; ++i) {
        if (strcmp(functions[i].name, name) == 0) {
            if (arity) *arity = functions[i].arity;
            return 1;
        }
    }
    return 0;
}

static int expr(const AstNode *n, char **names, size_t count,
                const FunctionSymbol *functions, size_t function_count) {
    if (!n) return 0;
    if (n->kind == AST_INTEGER) return 1;
    if (n->kind == AST_VARIABLE) {
        if (!has_name(names, count, n->name)) {
            fprintf(stderr, "CarsonCC: line %lu: undefined variable '%s'\n", (unsigned long)n->line, n->name);
            return 0;
        }
        return 1;
    }
    if (n->kind == AST_BINARY)
        return expr(n->left, names, count, functions, function_count) &&
               expr(n->right, names, count, functions, function_count);
    if (n->kind == AST_CALL) {
        size_t arity = 0;
        if (!find_function(functions, function_count, n->name, &arity)) {
            fprintf(stderr, "CarsonCC: line %lu: undefined function '%s'\n", (unsigned long)n->line, n->name);
            return 0;
        }
        if (n->child_count != arity) {
            fprintf(stderr, "CarsonCC: line %lu: function '%s' expects %lu argument(s), got %lu\n",
                    (unsigned long)n->line, n->name, (unsigned long)arity, (unsigned long)n->child_count);
            return 0;
        }
        if (arity > 6) {
            fprintf(stderr, "CarsonCC: line %lu: functions currently support at most 6 arguments\n", (unsigned long)n->line);
            return 0;
        }
        int ok = 1;
        for (size_t i = 0; i < n->child_count; ++i)
            if (!expr(n->children[i], names, count, functions, function_count)) ok = 0;
        return ok;
    }
    return 0;
}

static int function_check(const AstNode *f, const FunctionSymbol *functions, size_t function_count) {
    char **names = NULL;
    size_t count = 0;
    int ok = 1;

    for (size_t i = 0; i < f->parameter_count; ++i) {
        if (has_name(names, count, f->parameters[i])) {
            fprintf(stderr, "CarsonCC: line %lu: parameter '%s' already declared\n", (unsigned long)f->line, f->parameters[i]);
            ok = 0;
        }
        char **p = realloc(names, (count + 1) * sizeof(*p));
        if (!p) { fprintf(stderr, "CarsonCC: out of memory\n"); free(names); return 0; }
        names = p;
        names[count++] = f->parameters[i];
    }

    for (size_t i = 0; i < f->child_count; ++i) {
        const AstNode *n = f->children[i];
        if (n->kind == AST_LET) {
            if (has_name(names, count, n->name)) {
                fprintf(stderr, "CarsonCC: line %lu: variable '%s' already declared\n", (unsigned long)n->line, n->name);
                ok = 0;
            }
            if (!expr(n->left, names, count, functions, function_count)) ok = 0;
            char **p = realloc(names, (count + 1) * sizeof(*p));
            if (!p) { fprintf(stderr, "CarsonCC: out of memory\n"); free(names); return 0; }
            names = p;
            names[count++] = n->name;
        } else if (n->kind == AST_RETURN) {
            if (!expr(n->left, names, count, functions, function_count)) ok = 0;
        }
    }

    free(names);
    return ok;
}

int semantic_check(const AstNode *ast) {
    if (!ast || ast->kind != AST_PROGRAM || ast->child_count == 0) return 0;
    FunctionSymbol *functions = calloc(ast->child_count, sizeof(*functions));
    if (!functions) { fprintf(stderr, "CarsonCC: out of memory\n"); return 0; }

    int ok = 1;
    size_t function_count = 0;
    int has_main = 0;

    for (size_t i = 0; i < ast->child_count; ++i) {
        const AstNode *f = ast->children[i];
        if (f->kind != AST_FUNCTION) { ok = 0; continue; }
        if (find_function(functions, function_count, f->name, NULL)) {
            fprintf(stderr, "CarsonCC: line %lu: function '%s' already declared\n", (unsigned long)f->line, f->name);
            ok = 0;
        } else {
            functions[function_count++] = (FunctionSymbol){f->name, f->parameter_count};
        }
        if (strcmp(f->name, "main") == 0) {
            has_main = 1;
            if (f->parameter_count != 0) {
                fprintf(stderr, "CarsonCC: line %lu: main must not take parameters\n", (unsigned long)f->line);
                ok = 0;
            }
        }
    }

    if (!has_main) {
        fprintf(stderr, "CarsonCC: program must define fn main()\n");
        ok = 0;
    }

    for (size_t i = 0; i < ast->child_count; ++i)
        if (!function_check(ast->children[i], functions, function_count)) ok = 0;

    free(functions);
    return ok;
}
