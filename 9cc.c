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
        for (Var *var = prog->locals; var; var = var->next)
        {
            offset += 8;
        }
        int i = 0;
        for (Var *var = prog->locals; var; var = var->next)
        {
            var->offset = offset - 8 * i;
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
