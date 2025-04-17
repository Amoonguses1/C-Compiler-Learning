#include "9cc.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "invalid args\n");
        return 1;
    }

    // tokenize and parse.
    user_input = argv[1];
    token = tokenize();
    Function *prog = program();

    for (Function *fn = prog; fn; fn = fn->next)
    {
        int offset = 0;
        for (VarList *vl = fn->locals; vl; vl = vl->next)
        {
            offset += 8;
        }
        int i = 0;
        for (VarList *vl = fn->locals; vl; vl = vl->next)
        {
            vl->var->offset = offset - 8 * i;
            i++;
        }
        if (offset % 16)
        {
            offset += 8;
        }
        fn->stack_size = offset;
    }

    codegen(prog);
}
