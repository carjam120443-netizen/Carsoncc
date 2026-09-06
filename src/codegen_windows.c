#include "codegen.h"
#include <stdio.h>
#include <string.h>

typedef struct { FILE *out; const AstNode *function; } WinGen;

static int offset_of(const WinGen *g, const char *name) {
    int off = -8;
    for (size_t i = 0; i < g->function->parameter_count; ++i) {
        if (strcmp(g->function->parameters[i], name) == 0)
            return -8 - (int)(i * 8);
    }
    for (size_t i = 0; i < g->function->child_count; ++i) {
        const AstNode *n = g->function->children[i];
        if (n->kind == AST_LET) {
            if (strcmp(n->name, name) == 0)
                return off - (int)(g->function->parameter_count * 8);
            off -= 8;
        }
    }
    return 0;
}

static int locals(const AstNode *f) {
    int n = 0;
    for (size_t i = 0; i < f->child_count; ++i)
        if (f->children[i]->kind == AST_LET) ++n;
    return n;
}

static int expr(WinGen *g, const AstNode *n) {
    if (n->kind == AST_INTEGER) {
        fprintf(g->out, "    mov rax, %lld\\n", (long long)n->integer_value);
        return 1;
    }
    if (n->kind == AST_VARIABLE) {
        fprintf(g->out, "    mov rax, QWORD PTR [rbp%+d]\\n", offset_of(g, n->name));
        return 1;
    }
    if (n->kind == AST_CALL) {
        static const char *regs[] = {"rcx", "rdx", "r8", "r9"};
        if (n->child_count > 4) {
            fprintf(stderr, "CarsonCC: Windows x64 backend supports at most 4 call arguments\\n");
            return 0;
        }
        for (size_t i = n->child_count; i > 0; --i) {
            if (!expr(g, n->children[i - 1])) return 0;
            fprintf(g->out, "    push rax\\n");
        }
        for (size_t i = 0; i < n->child_count; ++i)
            fprintf(g->out, "    pop %s\\n", regs[i]);
        fprintf(g->out, "    sub rsp, 32\\n");
        fprintf(g->out, "    call %s\\n", n->name);
        fprintf(g->out, "    add rsp, 32\\n");
        return 1;
    }
    if (!expr(g, n->left)) return 0;
    fprintf(g->out, "    push rax\\n");
    if (!expr(g, n->right)) return 0;
    fprintf(g->out, "    mov r10, rax\\n");
    fprintf(g->out, "    pop rax\\n");
    switch (n->op) {
        case OP_ADD: fprintf(g->out, "    add rax, r10\\n"); break;
        case OP_SUB: fprintf(g->out, "    sub rax, r10\\n"); break;
        case OP_MUL: fprintf(g->out, "    imul rax, r10\\n"); break;
        case OP_DIV: fprintf(g->out, "    cqo\\n    idiv r10\\n"); break;
    }
    return 1;
}

static int function_codegen(FILE *out, const AstNode *f) {
    if (f->parameter_count > 4) {
        fprintf(stderr, "CarsonCC: Windows x64 backend supports at most 4 parameters in this release\\n");
        return 0;
    }
    int slots = (int)(f->parameter_count + locals(f));
    int bytes = ((slots * 8 + 15) / 16) * 16;
    WinGen g = {out, f};
    fprintf(out, "%s PROC\\n", f->name);
    fprintf(out, "    push rbp\\n    mov rbp, rsp\\n");
    if (bytes) fprintf(out, "    sub rsp, %d\\n", bytes);

    static const char *regs[] = {"rcx", "rdx", "r8", "r9"};
    for (size_t i = 0; i < f->parameter_count; ++i)
        fprintf(out, "    mov QWORD PTR [rbp%+d], %s\\n", offset_of(&g, f->parameters[i]), regs[i]);

    for (size_t i = 0; i < f->child_count; ++i) {
        const AstNode *x = f->children[i];
        if (x->kind == AST_LET) {
            if (!expr(&g, x->left)) return 0;
            fprintf(out, "    mov QWORD PTR [rbp%+d], rax\\n", offset_of(&g, x->name));
        } else if (x->kind == AST_RETURN) {
            if (!expr(&g, x->left)) return 0;
            fprintf(out, "    leave\\n    ret\\n");
        }
    }
    fprintf(out, "    xor eax, eax\\n    leave\\n    ret\\n%s ENDP\\n", f->name);
    return 1;
}

int codegen_windows_x86_64(const AstNode *program, const char *output_path) {
    FILE *out = fopen(output_path, "w");
    if (!out) { perror(output_path); return 0; }
    fprintf(out, ".code\\n");
    for (size_t i = 0; i < program->child_count; ++i) {
        if (!function_codegen(out, program->children[i])) {
            fclose(out);
            return 0;
        }
    }
    fprintf(out, "END\\n");
    fclose(out);
    return 1;
}
