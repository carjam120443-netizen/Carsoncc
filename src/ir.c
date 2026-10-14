#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { IRFunction *f; int failed; } Lower;

static char *copy_string(const char *s) {
    size_t n; char *p;
    if (!s) return NULL;
    n = strlen(s) + 1; p = (char *)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}
static void free_instruction(IRInstruction *in) {
    free(in->callee); free(in->arguments);
}
void ir_init(void) { /* Compatibility entry point; IRProgram owns its allocations. */ }
void ir_free(IRProgram *p) {
    size_t i, j;
    if (!p) return;
    for (i=0; i<p->function_count; ++i) {
        IRFunction *f=&p->functions[i];
        free(f->name);
        for (j=0; j<f->slot_count; ++j) free(f->slot_names[j]);
        free(f->slot_names);
        for (j=0; j<f->instruction_count; ++j) free_instruction(&f->instructions[j]);
        free(f->instructions);
    }
    free(p->functions); memset(p,0,sizeof(*p));
}
static int append(IRFunction *f, IRInstruction in) {
    IRInstruction *p=(IRInstruction *)realloc(f->instructions,(f->instruction_count+1)*sizeof(*p));
    if (!p) return 0;
    f->instructions=p; f->instructions[f->instruction_count++]=in; return 1;
}
static int add_slot(IRFunction *f,const char *name) {
    char **p; char *s=copy_string(name);
    if (!s) return 0;
    p=(char **)realloc(f->slot_names,(f->slot_count+1)*sizeof(*p));
    if (!p) { free(s); return 0; }
    f->slot_names=p; f->slot_names[f->slot_count++]=s; return 1;
}
static int slot_of(const IRFunction *f,const char *name) {
    size_t i;
    for(i=0;i<f->slot_count;i++) if(strcmp(f->slot_names[i],name)==0) return (int)i;
    return -1;
}
static int emit_value(Lower *c,IROp op,size_t line,int a,int b,long imm,size_t slot) {
    IRInstruction in; int id=c->f->next_value++;
    memset(&in,0,sizeof(in)); in.op=op; in.line=line; in.result=id; in.a=a; in.b=b; in.immediate=imm; in.slot=slot;
    if (!append(c->f,in)) { c->failed=1; return -1; }
    return id;
}
static int emit_effect(Lower *c,IROp op,size_t line,int a,size_t slot) {
    IRInstruction in; memset(&in,0,sizeof(in)); in.op=op; in.line=line; in.result=-1; in.a=a; in.b=-1; in.slot=slot;
    if (!append(c->f,in)) { c->failed=1; return 0; } return 1;
}
static int lower_expr(Lower *c,const AstNode *n);
static int lower_call(Lower *c,const AstNode *n) {
    IRInstruction in; size_t i;
    memset(&in,0,sizeof(in)); in.op=IR_CALL; in.line=n->line; in.result=-1; in.a=in.b=-1;
    in.callee=copy_string(n->name); in.argument_count=n->child_count;
    if (!in.callee) goto fail;
    if (in.argument_count) {
        in.arguments=(int *)malloc(in.argument_count*sizeof(int));
        if (!in.arguments) goto fail;
        for(i=0;i<in.argument_count;i++) {
            in.arguments[i]=lower_expr(c,n->children[i]);
            if(in.arguments[i]<0 || c->failed) goto fail;
        }
    }
    in.result=c->f->next_value++;
    if(!append(c->f,in)) goto fail;
    return in.result;
fail:
    free_instruction(&in); c->failed=1; return -1;
}
static int lower_expr(Lower *c,const AstNode *n) {
    int a,b,s; IROp op;
    if(!n || c->failed) { c->failed=1; return -1; }
    switch(n->kind) {
    case AST_INTEGER: return emit_value(c,IR_CONST,n->line,-1,-1,n->integer_value,0);
    case AST_VARIABLE:
        s=slot_of(c->f,n->name);
        if(s<0) { fprintf(stderr,"CarsonCC IR: line %lu: unknown variable '%s'\n",(unsigned long)n->line,n->name); c->failed=1; return -1; }
        return emit_value(c,IR_LOAD,n->line,-1,-1,0,(size_t)s);
    case AST_BINARY:
        a=lower_expr(c,n->left); b=lower_expr(c,n->right);
        if(a<0 || b<0 || c->failed) return -1;
        switch(n->op) { case OP_ADD:op=IR_ADD;break; case OP_SUB:op=IR_SUB;break; case OP_MUL:op=IR_MUL;break; case OP_DIV:op=IR_DIV;break; default:c->failed=1;return -1; }
        return emit_value(c,op,n->line,a,b,0,0);
    case AST_CALL: return lower_call(c,n);
    default: fprintf(stderr,"CarsonCC IR: line %lu: expected expression\n",(unsigned long)n->line); c->failed=1; return -1;
    }
}
static int lower_function(const AstNode *ast,IRFunction *f) {
    size_t i; Lower c; int v;
    memset(f,0,sizeof(*f)); f->name=copy_string(ast->name); f->parameter_count=ast->parameter_count;
    if(!f->name) return 0;
    for(i=0;i<ast->parameter_count;i++) if(!add_slot(f,ast->parameters[i])) return 0;
    /* Reserve stable slots for every local before lowering expressions. */
    for(i=0;i<ast->child_count;i++) if(ast->children[i]->kind==AST_LET) {
        if(slot_of(f,ast->children[i]->name)>=0 || !add_slot(f,ast->children[i]->name)) return 0;
    }
    c.f=f; c.failed=0;
    for(i=0;i<ast->child_count;i++) {
        const AstNode *s=ast->children[i];
        if(s->kind==AST_LET) {
            v=lower_expr(&c,s->left);
            if(v<0 || !emit_effect(&c,IR_STORE,s->line,v,(size_t)slot_of(f,s->name))) return 0;
        } else if(s->kind==AST_RETURN) {
            v=lower_expr(&c,s->left);
            if(v<0 || !emit_effect(&c,IR_RETURN,s->line,v,0)) return 0;
        } else { c.failed=1; return 0; }
        if(c.failed) return 0;
    }
    if(!f->instruction_count || f->instructions[f->instruction_count-1].op!=IR_RETURN) {
        v=emit_value(&c,IR_CONST,0,-1,-1,0,0);
        if(v<0 || !emit_effect(&c,IR_RETURN,0,v,0)) return 0;
    }
    return !c.failed;
}
static int defined_value(const unsigned char *defs,size_t count,int id) {
    return id>=0 && (size_t)id<count && defs[id];
}
int ir_verify(const IRProgram *p,FILE *diag) {
    size_t i,j,k;
    if(!p) return 0;
    for(i=0;i<p->function_count;i++) {
        const IRFunction *f=&p->functions[i]; size_t count=(size_t)f->next_value;
        unsigned char *defs; int has_return=0;
        if(!f->name || f->next_value<0 || f->parameter_count>f->slot_count || (f->slot_count && !f->slot_names) || (f->instruction_count && !f->instructions)) { if(diag)fprintf(diag,"CarsonCC IR: invalid function metadata\n"); return 0; }
        defs=count?(unsigned char *)calloc(count,1):NULL;
        if(count && !defs) return 0;
        for(j=0;j<f->instruction_count;j++) {
            const IRInstruction *in=&f->instructions[j]; int needs_a=(in->op==IR_STORE || in->op==IR_ADD || in->op==IR_SUB || in->op==IR_MUL || in->op==IR_DIV || in->op==IR_RETURN);
            int needs_b=(in->op==IR_ADD || in->op==IR_SUB || in->op==IR_MUL || in->op==IR_DIV);
            if((needs_a&&!defined_value(defs,count,in->a)) || (needs_b&&!defined_value(defs,count,in->b)) ||
               ((in->op==IR_LOAD || in->op==IR_STORE) && in->slot>=f->slot_count)) {
                if(diag)fprintf(diag,"CarsonCC IR: invalid operand in %s instruction %lu\n",f->name,(unsigned long)j);
                free(defs); return 0;
            }
            if(in->op==IR_CALL) {
                if(!in->callee || (in->argument_count && !in->arguments)) { if(diag)fprintf(diag,"CarsonCC IR: invalid call metadata in %s\n",f->name); free(defs); return 0; }
                for(k=0;k<in->argument_count;k++) if(!defined_value(defs,count,in->arguments[k])) {
                    if(diag)fprintf(diag,"CarsonCC IR: invalid call argument in %s\n",f->name);
                    free(defs); return 0;
                }
            }
            if(in->op==IR_RETURN) has_return=1;
            if(in->result>=0) {
                if((size_t)in->result>=count || defs[in->result]) { free(defs); return 0; }
                defs[in->result]=1;
            } else if(in->op==IR_CONST || in->op==IR_LOAD || in->op==IR_ADD || in->op==IR_SUB || in->op==IR_MUL || in->op==IR_DIV || in->op==IR_CALL) {
                free(defs); return 0;
            }
        }
        free(defs);
        if(!has_return) { if(diag)fprintf(diag,"CarsonCC IR: function %s has no return\n",f->name); return 0; }
    }
    return 1;
}
static const char *op_name(IROp op) {
    switch(op) { case IR_CONST:return "const"; case IR_LOAD:return "load"; case IR_STORE:return "store"; case IR_ADD:return "add"; case IR_SUB:return "sub"; case IR_MUL:return "mul"; case IR_DIV:return "div"; case IR_CALL:return "call"; case IR_RETURN:return "return"; }
    return "invalid";
}
void ir_dump(const IRProgram *p,FILE *out) {
    size_t i,j,k;
    if(!p || !out) return;
    fprintf(out,"Carson IR v1\n");
    for(i=0;i<p->function_count;i++) {
        const IRFunction *f=&p->functions[i];
        fprintf(out,"\nfunction %s (",f->name);
        for(j=0;j<f->parameter_count;j++) fprintf(out,"%s%s",j?", ":"",f->slot_names[j]);
        fprintf(out,") slots=%lu {\n",(unsigned long)f->slot_count);
        for(j=0;j<f->instruction_count;j++) {
            const IRInstruction *in=&f->instructions[j];
            fprintf(out,"  "); if(in->result>=0)fprintf(out,"%%t%d = ",in->result);
            fprintf(out,"%s",op_name(in->op));
            switch(in->op) {
            case IR_CONST:fprintf(out," %ld",in->immediate);break;
            case IR_LOAD:fprintf(out," $%lu ; %s",(unsigned long)in->slot,f->slot_names[in->slot]);break;
            case IR_STORE:fprintf(out," $%lu, %%t%d ; %s",(unsigned long)in->slot,in->a,f->slot_names[in->slot]);break;
            case IR_ADD:case IR_SUB:case IR_MUL:case IR_DIV:fprintf(out," %%t%d, %%t%d",in->a,in->b);break;
            case IR_CALL:fprintf(out," %s(",in->callee);for(k=0;k<in->argument_count;k++)fprintf(out,"%s%%t%d",k?", ":"",in->arguments[k]);fprintf(out,")");break;
            case IR_RETURN:fprintf(out," %%t%d",in->a);break;
            }
            if(in->line)fprintf(out,"  ; line %lu",(unsigned long)in->line);
            fputc('\n',out);
        }
        fprintf(out,"}\n");
    }
}


int ir_lower(const AstNode *ast, IRProgram *out) {
    size_t i;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!ast || ast->kind != AST_PROGRAM || !ast->child_count) {
        fprintf(stderr, "CarsonCC IR: expected a non-empty program AST\n");
        return 0;
    }
    out->functions = (IRFunction *)calloc(ast->child_count, sizeof(*out->functions));
    if (!out->functions) {
        fprintf(stderr, "CarsonCC IR: out of memory\n");
        return 0;
    }
    out->function_count = ast->child_count;
    for (i = 0; i < ast->child_count; ++i) {
        if (!ast->children[i] || ast->children[i]->kind != AST_FUNCTION ||
            !lower_function(ast->children[i], &out->functions[i])) {
            fprintf(stderr, "CarsonCC IR: failed to lower function %lu\n", (unsigned long)i);
            ir_free(out);
            return 0;
        }
    }
    if (!ir_verify(out, stderr)) {
        fprintf(stderr, "CarsonCC IR: verification failed\n");
        ir_free(out);
        return 0;
    }
    return 1;
}
