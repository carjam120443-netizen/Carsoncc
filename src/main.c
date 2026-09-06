#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "semantic.h"
#include "codegen.h"

static char *read_file(const char *path, size_t *size_out) {
    FILE *f=fopen(path,"rb"); if(!f){perror(path);return NULL;}
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return NULL;} long n=ftell(f); if(n<0){fclose(f);return NULL;} rewind(f);
    char *buf=malloc((size_t)n+1); if(!buf){fclose(f);return NULL;}
    if(fread(buf,1,(size_t)n,f)!=(size_t)n){free(buf);fclose(f);return NULL;} buf[n]=0; fclose(f); if(size_out)*size_out=(size_t)n; return buf;
}

static void usage(void){
    fprintf(stderr,"usage: carsoncc <input.car> [-o output] [-S] [-target TARGET]\n"
                   "  -S             emit assembly instead of linking\n"
                   "  -o FILE        output executable or assembly file\n"
                   "  -target TARGET target backend: host or windows\n");
}

int main(int argc,char **argv){
    if(argc<2){usage();return EXIT_FAILURE;}
    const char *input=NULL,*output=NULL,*target="host";int assembly_only=0;
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"-S")==0) assembly_only=1;
        else if(strcmp(argv[i],"-o")==0){if(++i>=argc){usage();return EXIT_FAILURE;}output=argv[i];}
        else if(strcmp(argv[i],"-target")==0){if(++i>=argc){usage();return EXIT_FAILURE;}target=argv[i];}
        else if(argv[i][0]=='-'){fprintf(stderr,"CarsonCC: unknown option '%s'\n",argv[i]);return EXIT_FAILURE;}
        else if(!input)input=argv[i];else{fprintf(stderr,"CarsonCC: multiple input files are not supported yet\n");return EXIT_FAILURE;}
    }
    if(!input){usage();return EXIT_FAILURE;}
    int windows_target = strcmp(target,"windows")==0 || strcmp(target,"win64")==0;
    if(!windows_target && strcmp(target,"host")!=0){fprintf(stderr,"CarsonCC: unknown target '%s'\n",target);return EXIT_FAILURE;}

    char default_asm[512];char default_out[512];
    if(!output){snprintf(default_out,sizeof(default_out),"a.out");output=default_out;}
    snprintf(default_asm,sizeof(default_asm),"%s.carsoncc.%s",output,windows_target?"asm":"s");
    const char *asm_path=assembly_only?output:default_asm;
    size_t size=0;char *source=read_file(input,&size);(void)size;if(!source)return EXIT_FAILURE;
    Parser parser;parser_init(&parser,source);AstNode *program=parser_parse(&parser);free(source);
    if(!program){fprintf(stderr,"CarsonCC: compilation failed during parsing\n");return EXIT_FAILURE;}
    if(!semantic_check(program)){ast_free(program);fprintf(stderr,"CarsonCC: compilation failed during semantic analysis\n");return EXIT_FAILURE;}

    int generated = windows_target ? codegen_windows_x86_64(program,asm_path) : codegen_x86_64(program,asm_path);
    ast_free(program);
    if(!generated){remove(asm_path);return EXIT_FAILURE;}
    if(assembly_only){printf("CarsonCC: wrote %s\n",asm_path);return EXIT_SUCCESS;}

    char command[1600];
    int rc;
    if(windows_target){
        char obj_path[512];
        snprintf(obj_path,sizeof(obj_path),"%s.carsoncc.obj",output);
        snprintf(command,sizeof(command),"ml64 /c /Fo\"%s\" \"%s\" && if not exist \"%s\" exit /b 1 && link /nologo /SUBSYSTEM:CONSOLE /ENTRY:main /OUT:\"%s\" \"%s\" && if not exist \"%s\" exit /b 1",obj_path,asm_path,obj_path,output,obj_path,output);
        rc=system(command);
        remove(obj_path);
    }else{
        snprintf(command,sizeof(command),"cc -x assembler '%s' -o '%s'",asm_path,output);
        rc=system(command);
    }
    remove(asm_path);
    if(rc!=0){fprintf(stderr,"CarsonCC: system linker failed for target %s\n",target);return EXIT_FAILURE;}
    printf("CarsonCC: built %s for %s\n",output,target);return EXIT_SUCCESS;
}
