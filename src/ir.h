#ifndef CARSONCC_IR_H
#define CARSONCC_IR_H

#include <stddef.h>
#include <stdio.h>
#include "ast.h"

/* Carson IR v1: three-address instructions and function-local temporary values. */
typedef enum { IR_CONST, IR_LOAD, IR_STORE, IR_ADD, IR_SUB, IR_MUL, IR_DIV, IR_CALL, IR_RETURN } IROp;
typedef struct {
    IROp op;
    size_t line, slot, argument_count;
    int result, a, b;
    long immediate;
    char *callee;
    int *arguments;
} IRInstruction;
typedef struct {
    char *name;
    char **slot_names; /* Parameters first, then local variables. */
    size_t parameter_count, slot_count, instruction_count;
    IRInstruction *instructions;
    int next_value;
} IRFunction;
typedef struct { IRFunction *functions; size_t function_count; } IRProgram;

void ir_init(void);
int ir_lower(const AstNode *ast, IRProgram *out);
int ir_verify(const IRProgram *program, FILE *diagnostics);
void ir_dump(const IRProgram *program, FILE *out);
void ir_free(IRProgram *program);

#endif
