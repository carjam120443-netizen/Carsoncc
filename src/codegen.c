#include "codegen.h"
#include <stdio.h>
#include <string.h>

typedef struct { FILE *out; const AstNode *function; int next_offset; } Gen;
static int offset_of(const Gen*g,const char*name){int off=-8;for(size_t i=0;i<g->function->child_count;i++){const AstNode*n=g->function->children[i];if(n->kind==AST_LET){if(strcmp(n->name,name)==0)return off;off-=8;}}return 0;}
static int locals(const AstNode*f){int n=0;for(size_t i=0;i<f->child_count;i++)if(f->children[i]->kind==AST_LET)n++;return n;}
static void expr(Gen*g,const AstNode*n){if(n->kind==AST_INTEGER){fprintf(g->out,"    mov rax, %ld\n",n->integer_value);return;}if(n->kind==AST_VARIABLE){fprintf(g->out,"    mov rax, QWORD PTR [rbp%d]\n",offset_of(g,n->name));return;}expr(g,n->left);fprintf(g->out,"    push rax\n");expr(g,n->right);fprintf(g->out,"    mov rcx, rax\n    pop rax\n");switch(n->op){case OP_ADD:fprintf(g->out,"    add rax, rcx\n");break;case OP_SUB:fprintf(g->out,"    sub rax, rcx\n");break;case OP_MUL:fprintf(g->out,"    imul rax, rcx\n");break;case OP_DIV:fprintf(g->out,"    cqo\n    idiv rcx\n");break;}}
static void function_codegen(FILE*out,const AstNode*f){int n=locals(f);int bytes=((n*8+15)/16)*16;Gen g={out,f,0};fprintf(out,".globl %s\n.type %s, @function\n%s:\n    push rbp\n    mov rbp, rsp\n",f->name,f->name,f->name);if(bytes)fprintf(out,"    sub rsp, %d\n",bytes);for(size_t i=0;i<f->child_count;i++){const AstNode*x=f->children[i];if(x->kind==AST_LET){expr(&g,x->left);int off=offset_of(&g,x->name);fprintf(out,"    mov QWORD PTR [rbp%d], rax\n",off);}else if(x->kind==AST_RETURN){expr(&g,x->left);fprintf(out,"    leave\n    ret\n");}}fprintf(out,"    xor eax, eax\n    leave\n    ret\n");}
int codegen_x86_64(const AstNode*program,const char*output_path){FILE*out=fopen(output_path,"w");if(!out){perror(output_path);return 0;}fprintf(out,".intel_syntax noprefix\n.text\n");for(size_t i=0;i<program->child_count;i++)function_codegen(out,program->children[i]);fclose(out);return 1;}
