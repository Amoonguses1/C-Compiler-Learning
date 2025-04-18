#include "9cc.h"

int labelseq = 0;
char *funcname;
char *argreg[] = {"x0", "x1", "x2", "x3", "x4", "x5"};

void gen(Node *node);

void gen_addr(Node *node)
{
    switch (node->kind)
    {
    case ND_VAR:
        printf("    sub x0, x29, %d\n", node->var->offset);
        printf("    str x0, [sp, -16]!\n");
        return;
    case ND_DEREF:
        gen(node->lhs);
        return;
    }

    error_tok(node->tok, "not an lvalue");
}

void load()
{
    printf("    ldr x0, [sp], 16\n");
    printf("    ldr x0, [x0]\n");
    printf("    str x0, [sp, -16]!\n");
}

void store()
{
    printf("    ldr x1, [sp], 16\n");
    printf("    ldr x0, [sp], 16\n");
    printf("    str x1, [x0]\n");
    printf("    str x1, [sp, -16]\n");
}

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    mov x0, %d\n", node->val);
        printf("    sub sp, sp, 16\n");
        printf("    str x0, [sp, 0]\n");
        return;
    case ND_EXPR_STMT:
        gen(node->lhs);
        printf("    add sp, sp, 16\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    ldr x0, [sp], 16\n");
        printf("    b .Lreturn.%s\n", funcname);
        return;
    case ND_ADDR:
        gen_addr(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        load();
        return;
    case ND_IF:
        int seq = labelseq++;
        if (node->els)
        {
            gen(node->cond);
            printf("    ldr x0, [sp, 0]\n");
            printf("    add sp, sp, 16\n");
            printf("    cbz x0, .Lelse%d\n", seq);
            gen(node->then);
            printf("    b .Lend%d\n", seq);
            printf(".Lelse%d:\n", seq);
            gen(node->els);
            printf(".Lend%d:\n", seq);
        }
        else
        {
            gen(node->cond);
            printf("    ldr x0, [sp, 0]\n");
            printf("    add sp, sp, 16\n");
            printf("    cbz x0, .Lend%d\n", seq);
            gen(node->then);
            printf(".Lend%d:\n", seq);
        }
        return;
    case ND_WHILE:
        seq = labelseq++;
        printf(".Lbegin%d:\n", seq);
        gen(node->cond);
        printf("    ldr x0, [sp, 0]\n");
        printf("    add sp, sp, 16\n");
        printf("    cbz x0, .Lend%d\n", seq);
        gen(node->then);
        printf("    b .Lbegin%d\n", seq);
        printf(".Lend%d:\n", seq);
        return;
    case ND_FOR:
        seq = labelseq++;
        if (node->init)
        {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", seq);
        if (node->cond)
        {
            gen(node->cond);
            printf("    ldr x0, [sp, 0]\n");
            printf("    add sp, sp, 16\n");
            printf("    cbz x0, .Lend%d\n", seq);
        }
        gen(node->then);
        if (node->inc)
        {
            gen(node->inc);
        }
        printf("    b .Lbegin%d\n", seq);
        printf(".Lend%d:\n", seq);
        return;
    case ND_BLOCK:
        for (Node *n = node->body; n; n = n->next)
        {
            gen(n);
        }
        return;
    case ND_FUNCALL:
        int nargs = 0;
        for (Node *arg = node->args; arg; arg = arg->next)
        {
            gen(arg);
            nargs++;
        }
        for (int i = nargs - 1; i >= 0; i--)
        {
            printf("    ldr %s, [sp, 0]\n", argreg[i]);
            printf("    add sp, sp, 16\n");
        }
        printf("    stp x29, x30, [sp, -16]!\n");
        printf("    mov x29, sp\n");
        printf("    bl %s\n", node->funcname);
        printf("    ldp x29, x30, [sp], 16\n");
        printf("    sub sp, sp, #16\n");
        printf("    str x0, [sp]\n");
        return;
    case ND_VAR:
        gen_addr(node);
        load();
        return;
    case ND_ASSIGN:
        gen_addr(node->lhs);
        gen(node->rhs);
        store();
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // load rhs value
    printf("    ldr x1, [sp, 0]\n");
    printf("    add sp, sp, 16\n");

    // load lhs value
    printf("    ldr x0, [sp, 0]\n");
    printf("    add sp, sp, 16\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add x0, x0, x1\n");
        break;
    case ND_SUB:
        printf("    sub x0, x0, x1\n");
        break;
    case ND_MUL:
        printf("    mul x0, x0, x1\n");
        break;
    case ND_DIV:
        printf("    sdiv x0, x0, x1\n");
        break;
    case ND_EQ:
        printf("    cmp x0, x1\n");
        printf("    cset x0, eq\n");
        break;
    case ND_NE:
        printf("    cmp x0, x1\n");
        printf("    cset x0, ne\n");
        break;
    case ND_LE:
        printf("    cmp x0, x1\n");
        printf("    cset x0, le\n");
        break;
    case ND_LT:
        printf("    cmp x0, x1\n");
        printf("    cset x0, lt\n");
    }

    printf("    sub sp, sp, 16\n");
    printf("    str x0, [sp, 0]\n");
}

void codegen(Function *prog)
{
    for (Function *fn = prog; fn; fn = fn->next)
    {
        printf(".globl %s\n", fn->name);
        printf("%s:\n", fn->name);
        funcname = fn->name;

        // Prologue
        printf("    str x29, [sp, -16]!\n");
        printf("    mov x29, sp\n");
        printf("    sub sp, sp, %d\n", fn->stack_size);

        // Push arguments to the stack
        int i = 0;
        for (VarList *vl = fn->params; vl; vl = vl->next)
        {
            Var *var = vl->var;
            printf("    str %s, [x29, -%d]\n", argreg[i++], var->offset);
        }

        // code generation walking the AST.
        for (Node *n = fn->node; n; n = n->next)
        {
            gen(n);
            printf("    ldr x0, [sp, 0]\n");
            printf("    add sp, sp, 16\n");
        }

        printf(".Lreturn.%s:\n", funcname);
        printf("    mov sp, x29\n");
        printf("    ldr x29, [sp], 16\n");
        printf("    ret\n");
    }
}
