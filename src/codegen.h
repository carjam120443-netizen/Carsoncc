#ifndef CARSONCC_CODEGEN_H
#define CARSONCC_CODEGEN_H

/* Target code generation interface. The first backend will target x86-64. */
int codegen_x86_64(void *ir, const char *output_path);

#endif
