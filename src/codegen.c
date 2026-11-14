#include "codegen.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    FILE *out;
    const AstNode *function;
} Gen;

static int offset_of(const Gen *gen, const char *name) {
    int offset = -8;

    for (size_t i = 0; i < gen->function->parameter_count; ++i) {
        if (!strcmp(gen->function->parameters[i], name)) {
            return -8 - (int)(i * 8);
        }
    }

    for (size_t i = 0; i < gen->function->child_count; ++i) {
        const AstNode *node = gen->function->children[i];

        if (node->kind == AST_LET) {
            if (!strcmp(node->name, name)) {
                return offset - (int)(gen->function->parameter_count * 8);
            }
            offset -= 8;
        }
    }

    return 0;
}

static int locals(const AstNode *function) {
    int count = 0;

    for (size_t i = 0; i < function->child_count; ++i) {
        if (function->children[i]->kind == AST_LET) {
            count++;
        }
    }

    return count;
}

static void expr(Gen *gen, const AstNode *node) {
    if (node->kind == AST_INTEGER) {
        fprintf(gen->out, "    mov rax, %ld\n", node->integer_value);
        return;
    }

    if (node->kind == AST_VARIABLE) {
        fprintf(gen->out, "    mov rax, QWORD PTR [rbp%d]\n",
                offset_of(gen, node->name));
        return;
    }

    if (node->kind == AST_CALL) {
        static const char *registers[] = {
            "rdi", "rsi", "rdx", "rcx", "r8", "r9"
        };

        for (size_t i = node->child_count; i > 0; --i) {
            expr(gen, node->children[i - 1]);
            fprintf(gen->out, "    push rax\n");
        }

        for (size_t i = 0; i < node->child_count && i < 6; ++i) {
            fprintf(gen->out, "    pop %s\n", registers[i]);
        }

        if (node->child_count > 6) {
            fprintf(gen->out, "    add rsp, %lu\n",
                    (unsigned long)((node->child_count - 6) * 8));
        }

        fprintf(gen->out, "    call %s\n", node->name);
        return;
    }

    expr(gen, node->left);
    fprintf(gen->out, "    push rax\n");
    expr(gen, node->right);
    fprintf(gen->out, "    mov rcx, rax\n    pop rax\n");

    switch (node->op) {
        case OP_ADD:
            fprintf(gen->out, "    add rax,rcx\n");
            break;
        case OP_SUB:
            fprintf(gen->out, "    sub rax,rcx\n");
            break;
        case OP_MUL:
            fprintf(gen->out, "    imul rax,rcx\n");
            break;
        case OP_DIV:
            fprintf(gen->out, "    cqo\n    idiv rcx\n");
            break;
    }
}

static void function_codegen(FILE *out, const AstNode *function) {
    int slots = (int)(function->parameter_count + locals(function));
    int bytes = ((slots * 8 + 15) / 16) * 16;
    Gen gen = {out, function};

    fprintf(out,
            ".globl %s\n.type %s,@function\n%s:\n"
            "    push rbp\n    mov rbp,rsp\n",
            function->name, function->name, function->name);

    if (bytes) {
        fprintf(out, "    sub rsp,%d\n", bytes);
    }

    static const char *registers[] = {
        "rdi", "rsi", "rdx", "rcx", "r8", "r9"
    };

    for (size_t i = 0; i < function->parameter_count; ++i) {
        if (i < 6) {
            fprintf(out, "    mov QWORD PTR [rbp%d],%s\n",
                    offset_of(&gen, function->parameters[i]), registers[i]);
        }
    }

    for (size_t i = 0; i < function->child_count; ++i) {
        const AstNode *statement = function->children[i];

        if (statement->kind == AST_LET) {
            expr(&gen, statement->left);
            fprintf(out, "    mov QWORD PTR [rbp%d],rax\n",
                    offset_of(&gen, statement->name));
        } else if (statement->kind == AST_RETURN) {
            expr(&gen, statement->left);
            fprintf(out, "    leave\n    ret\n");
        }
    }

    fprintf(out, "    xor eax,eax\n    leave\n    ret\n");
}

int codegen_x86_64(const AstNode *program, const char *path) {
    FILE *out = fopen(path, "w");

    if (!out) {
        perror(path);
        return 0;
    }

    fprintf(out, ".intel_syntax noprefix\n.text\n");

    for (size_t i = 0; i < program->child_count; ++i) {
        function_codegen(out, program->children[i]);
    }

    fprintf(out, ".section .note.GNU-stack,\"\",@progbits\n");
    fclose(out);
    return 1;
}
