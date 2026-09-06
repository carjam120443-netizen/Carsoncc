#include "ast.h"
#include <stdlib.h>
#include <string.h>

static char *dup_text(const char *s){size_t n=strlen(s)+1;char*p=malloc(n);if(p)memcpy(p,s,n);return p;}
AstNode *ast_new(AstKind kind,size_t line){AstNode*n=calloc(1,sizeof(*n));if(n){n->kind=kind;n->line=line;}return n;}
int ast_add_child(AstNode*p,AstNode*c){AstNode**x=realloc(p->children,(p->child_count+1)*sizeof(*x));if(!x)return 0;p->children=x;p->children[p->child_count++]=c;return 1;}
int ast_add_parameter(AstNode*f,const char*name){size_t index=f->parameter_count;char*text=dup_text(name);if(!text)return 0;char**x=realloc(f->parameters,(index+1)*sizeof(*x));if(!x){free(text);return 0;}x[index]=text;f->parameters=x;f->parameter_count=index+1;return 1;}
void ast_free(AstNode*n){if(!n)return;free(n->name);for(size_t i=0;i<n->parameter_count;i++)free(n->parameters[i]);free(n->parameters);ast_free(n->left);ast_free(n->right);for(size_t i=0;i<n->child_count;i++)ast_free(n->children[i]);free(n->children);free(n);}
