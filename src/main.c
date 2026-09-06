#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: carsoncc <file.car>\n");
        return EXIT_FAILURE;
    }

    printf("CarsonCC: compiling %s\n", argv[1]);
    printf("CarsonCC: lexer/parser pipeline is under construction.\n");
    return EXIT_SUCCESS;
}
