#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void advance_token(Parser*p){p->previous=p->current;p->current=lexer_next(&p->lexer);}
static int check(Parser*p,TokenKind k){return p->current.kind==k;}
static int match(Parser*p,TokenKind k){if(!check(p,k))return 0;advance_token(p);return 1;}
static void error(Parser*p,const char*m){if(!p->had_error)fprintf(stderr,"CarsonCC: line %lu: %s (got %s)\n",(unsigned long)p->current.line,m,token_kind_name(p->current.kind));p->had_error=1;}
static int consume(Parser*p,TokenKind k,const char*m){if(check(p,k)){advance_token(p);return 1;}error(p,m);return 0;}
static char*token_text(Token t){char*s=malloc(t.length+1);if(!s)return NULL;memcpy(s,t.start,t.length);s[t.length]=0;return s;}
static AstNode*expression(Parser*p);
static AstNode*primary(Parser*p){
 if(match(p,TOKEN_INTEGER)){AstNode*n=ast_new(AST_INTEGER,p->previous.line);if(n)n->integer_value=strtol(p->previous.start,NULL,10);return n;}
 if(match(p,TOKEN_IDENTIFIER)){Token name=p->previous;if(match(p,TOKEN_LPAREN)){AstNode*n=ast_new(AST_CALL,name.line);if(!n)return NULL;n->name=token_text(name);if(!check(p,TOKEN_RPAREN)){do{AstNode*a=expression(p);if(!a||!ast_add_child(n,a)){ast_free(a);ast_free(n);return NULL;}}while(match(p,TOKEN_COMMA));}consume(p,TOKEN_RPAREN,"expected ')' after arguments");return n;}AstNode*n=ast_new(AST_VARIABLE,name.line);if(n)n->name=token_text(name);return n;}
 if(match(p,TOKEN_LPAREN)){AstNode*n=expression(p);consume(p,TOKEN_RPAREN,"expected ')' after expression");return n;}
 error(p,"expected expression");return NULL;
}
static AstNode*factor(Parser*p){AstNode*l=primary(p);while(check(p,TOKEN_STAR)||check(p,TOKEN_SLASH)){Token op=p->current;advance_token(p);AstNode*r=primary(p);if(!l||!r){ast_free(l);ast_free(r);return NULL;}AstNode*n=ast_new(AST_BINARY,op.line);if(!n){ast_free(l);ast_free(r);return NULL;}n->left=l;n->right=r;n->op=op.kind==TOKEN_STAR?OP_MUL:OP_DIV;l=n;}return l;}
static AstNode*expression(Parser*p){AstNode*l=factor(p);while(check(p,TOKEN_PLUS)||check(p,TOKEN_MINUS)){Token op=p->current;advance_token(p);AstNode*r=factor(p);if(!l||!r){ast_free(l);ast_free(r);return NULL;}AstNode*n=ast_new(AST_BINARY,op.line);if(!n){ast_free(l);ast_free(r);return NULL;}n->left=l;n->right=r;n->op=op.kind==TOKEN_PLUS?OP_ADD:OP_SUB;l=n;}return l;}
static AstNode*statement(Parser*p){
 if(match(p,TOKEN_LET)){if(!check(p,TOKEN_IDENTIFIER)){error(p,"expected variable name after 'let'");return NULL;}Token name=p->current;advance_token(p);consume(p,TOKEN_EQUAL,"expected '=' after variable name");AstNode*v=expression(p);consume(p,TOKEN_SEMICOLON,"expected ';' after declaration");AstNode*n=ast_new(AST_LET,name.line);if(!n){ast_free(v);return NULL;}n->name=token_text(name);n->left=v;return n;}
 if(match(p,TOKEN_RETURN)){AstNode*v=expression(p);consume(p,TOKEN_SEMICOLON,"expected ';' after return value");AstNode*n=ast_new(AST_RETURN,p->previous.line);if(n)n->left=v;return n;}
 error(p,"expected statement");return NULL;
}
static AstNode*function(Parser*p){Token fn=p->previous;if(!check(p,TOKEN_IDENTIFIER)){error(p,"expected function name");return NULL;}Token name=p->current;advance_token(p);consume(p,TOKEN_LPAREN,"expected '(' after function name");AstNode*f=ast_new(AST_FUNCTION,fn.line);if(!f)return NULL;f->name=token_text(name);
 if(!check(p,TOKEN_RPAREN)){do{if(!check(p,TOKEN_IDENTIFIER)){error(p,"expected parameter name");ast_free(f);return NULL;}Token param=p->current;advance_token(p);char*s=token_text(param);if(!s||!ast_add_parameter(f,s)){free(s);ast_free(f);return NULL;}free(s);}while(match(p,TOKEN_COMMA));}
 consume(p,TOKEN_RPAREN,"expected ')' after parameter list");consume(p,TOKEN_LBRACE,"expected '{' before function body");while(!check(p,TOKEN_RBRACE)&&!check(p,TOKEN_EOF)){AstNode*s=statement(p);if(!s){ast_free(f);return NULL;}if(!ast_add_child(f,s)){ast_free(s);ast_free(f);return NULL;}}consume(p,TOKEN_RBRACE,"expected '}' after function body");return f;}
void parser_init(Parser*p,const char*s){memset(p,0,sizeof(*p));lexer_init(&p->lexer,s);p->current=lexer_next(&p->lexer);}
AstNode*parser_parse(Parser*p){AstNode*program=ast_new(AST_PROGRAM,1);if(!program)return NULL;while(!check(p,TOKEN_EOF)){if(!match(p,TOKEN_FN)){error(p,"expected 'fn' at top level");ast_free(program);return NULL;}AstNode*f=function(p);if(!f||!ast_add_child(program,f)){ast_free(f);ast_free(program);return NULL;}}if(p->had_error){ast_free(program);return NULL;}return program;}
