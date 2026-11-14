#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "semantic.h"
#include "codegen.h"

static char *read_file(const char *path, size_t *size_out) {
    FILE *file = fopen(path, "rb");

    if (!file) {
        perror(path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    long length = ftell(file);

    if (length < 0) {
        fclose(file);
        return NULL;
    }

    rewind(file);

    char *buffer = malloc((size_t)length + 1);

    if (!buffer) {
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, (size_t)length, file) != (size_t)length) {
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0';
    fclose(file);

    if (size_out) {
        *size_out = (size_t)length;
    }

    return buffer;
}

static void usage(void) {
    fprintf(stderr,
            "usage: carsoncc <input.car> [-o output] [-S] [-target TARGET]\n"
            "  -S             emit assembly instead of linking\n"
            "  -o FILE        output executable or assembly file\n"
            "  -target TARGET target backend: host or windows\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage();
        return EXIT_FAILURE;
    }

    const char *input = NULL;
    const char *output = NULL;
    const char *target = "host";
    int assembly_only = 0;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-S") == 0) {
            assembly_only = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (++i >= argc) {
                usage();
                return EXIT_FAILURE;
            }
            output = argv[i];
        } else if (strcmp(argv[i], "-target") == 0) {
            if (++i >= argc) {
                usage();
                return EXIT_FAILURE;
            }
            target = argv[i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "CarsonCC: unknown option '%s'\n", argv[i]);
            return EXIT_FAILURE;
        } else if (!input) {
            input = argv[i];
        } else {
            fprintf(stderr, "CarsonCC: multiple input files are not supported yet\n");
            return EXIT_FAILURE;
        }
    }

    if (!input) {
        usage();
        return EXIT_FAILURE;
    }

    int windows_target =
        strcmp(target, "windows") == 0 || strcmp(target, "win64") == 0;

    if (!windows_target && strcmp(target, "host") != 0) {
        fprintf(stderr, "CarsonCC: unknown target '%s'\n", target);
        return EXIT_FAILURE;
    }

    char default_asm[512];
    char default_output[512];

    if (!output) {
        snprintf(default_output, sizeof(default_output), "a.out");
        output = default_output;
    }

    snprintf(default_asm, sizeof(default_asm), "%s.carsoncc.%s",
             output, windows_target ? "asm" : "s");

    const char *assembly_path = assembly_only ? output : default_asm;
    size_t source_size = 0;
    char *source = read_file(input, &source_size);
    (void)source_size;

    if (!source) {
        return EXIT_FAILURE;
    }

    Parser parser;
    parser_init(&parser, source);

    AstNode *program = parser_parse(&parser);
    free(source);

    if (!program) {
        fprintf(stderr, "CarsonCC: compilation failed during parsing\n");
        return EXIT_FAILURE;
    }

    if (!semantic_check(program)) {
        ast_free(program);
        fprintf(stderr, "CarsonCC: compilation failed during semantic analysis\n");
        return EXIT_FAILURE;
    }

    int generated = windows_target
        ? codegen_windows_x86_64(program, assembly_path)
        : codegen_x86_64(program, assembly_path);

    ast_free(program);

    if (!generated) {
        remove(assembly_path);
        return EXIT_FAILURE;
    }

    if (assembly_only) {
        printf("CarsonCC: wrote %s\n", assembly_path);
        return EXIT_SUCCESS;
    }

    char command[1600];
    int result;

    if (windows_target) {
        char object_path[512];

        snprintf(object_path, sizeof(object_path), "%s.carsoncc.obj", output);
        snprintf(command, sizeof(command),
                 "ml64 /nologo /c /Fo\"%s\" \"%s\" && "
                 "link.exe /nologo /MACHINE:X64 /SUBSYSTEM:CONSOLE "
                 "/ENTRY:main /OUT:\"%s\" \"%s\"",
                 object_path, assembly_path, output, object_path);

        result = system(command);
        remove(object_path);

        if (result == 0) {
            FILE *built = fopen(output, "rb");

            if (!built) {
                fprintf(stderr,
                        "CarsonCC: Windows linker reported success but did not create '%s'\n",
                        output);
                result = 1;
            } else {
                fclose(built);
            }
        }
    } else {
        snprintf(command, sizeof(command),
                 "cc -x assembler '%s' -o '%s'", assembly_path, output);
        result = system(command);
    }

    remove(assembly_path);

    if (result != 0) {
        fprintf(stderr, "CarsonCC: system linker failed for target %s\n", target);
        return EXIT_FAILURE;
    }

    printf("CarsonCC: built %s for %s\n", output, target);
    return EXIT_SUCCESS;
}
