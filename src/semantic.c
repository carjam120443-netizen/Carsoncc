#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int has_name(char **names, size_t count, const char *name) { for(size_t i=0;i<count;i++) if(strcmp(names[i],name)==0) return 1; return 0; }
static int expr(const AstNode*n,char**names,size_t count){if(!n)return 0;if(n->kind==AST_INTEGER)return 1;if(n->kind==AST_VARIABLE){if(!has_name(names,count,n->name)){fprintf(stderr,"CarsonCC: line %lu: undefined variable '%s'\n",(unsigned long)n->line,n->name);return 0;}return 1;}if(n->kind==AST_BINARY)return expr(n->left,names,count)&&expr(n->right,names,count);return 0;}
static int function_check(const AstNode*f){char**names=NULL;size_t count=0;int ok=1;for(size_t i=0;i<f->child_count;i++){const AstNode*n=f->children[i];if(n->kind==AST_LET){if(has_name(names,count,n->name)){fprintf(stderr,"CarsonCC: line %lu: variable '%s' already declared\n",(unsigned long)n->line,n->name);ok=0;}if(!expr(n->left,names,count))ok=0;char**p=realloc(names,(count+1)*sizeof(*p));if(!p){fprintf(stderr,"CarsonCC: out of memory\n");free(names);return 0;}names=p;names[count++]=n->name;}else if(n->kind==AST_RETURN){if(!expr(n->left,names))ok=0;}}
    free(names); return ok;}
int semantic_check(const AstNode*ast){if(!ast||ast->kind!=AST_PROGRAM)return 0;int ok=1;for(size_t i=0;i<ast->child_count;i++)if(!function_check(ast->children[i]))ok=0;return ok;}
