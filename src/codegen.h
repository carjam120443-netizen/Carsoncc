#ifndef CARSONCC_CODEGEN_H
#define CARSONCC_CODEGEN_H
#include "ast.h"
int codegen_x86_64(const AstNode *program, const char *output_path);
int codegen_windows_x86_64(const AstNode *program, const char *output_path);
#endif
