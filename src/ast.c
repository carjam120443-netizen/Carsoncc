#include "ast.h"
#include <stdlib.h>
#include <string.h>

AstNode *ast_new(AstKind kind,size_t line){AstNode*n=calloc(1,sizeof(*n));if(n){n->kind=kind;n->line=line;}return n;}
int ast_add_child(AstNode*p,AstNode*c){AstNode**x=realloc(p->children,(p->child_count+1)*sizeof(*x));if(!x)return 0;p->children=x;p->children[p->child_count++]=c;return 1;}
int ast_add_parameter(AstNode*f,const char*name){char**x=realloc(f->parameters,(f->parameter_count+1)*sizeof(*x));if(!x)return 0;x[f->parameter_count]=strdup(name);if(!x[f->parameter_count])return 0;f->parameters=x;f->parameter_count++;return 1;}
void ast_free(AstNode*n){if(!n)return;free(n->name);for(size_t i=0;i<n->parameter_count;i++)free(n->parameters[i]);free(n->parameters);ast_free(n->left);ast_free(n->right);for(size_t i=0;i<n->child_count;i++)ast_free(n->children[i]);free(n->children);free(n);}
