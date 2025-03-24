#include "9cc.h"

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("    mov x0, %d\n", node->val);
        printf("    str x0, [sp, 0]\n");
        printf("    add sp, sp, 16\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // load rhs value
    printf("    sub sp, sp, 16\n");
    printf("    ldr x1, [sp, 0]\n");

    // load lhs value
    printf("    sub sp, sp, 16\n");
    printf("    ldr x0, [sp, 0]\n");

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

    printf("    str x0, [sp, 0]\n");
    printf("    add sp, sp, 16\n");
}

void codegen(Node *node)
{
    // assembler start
    printf(".globl main\n");
    printf("main:\n");

    gen(node);

    // code generation walking the AST.
    gen(node);

    // pop the stack top and treat as return value
    printf("    sub sp, sp, 16\n");
    printf("    ldr x0, [sp, 0]\n");
    printf("    ret\n");
    return 0;
}
