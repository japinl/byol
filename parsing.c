#include "mpc.h"

#ifdef _WIN32

char *readline(char *prompt)
{
    static char buffer[2048];
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    return strdup(buffer);
}

void add_history(char *unused)
{
    (void)(unused);
}

#else

#include <editline/readline.h>

#endif

int main(int argc, char **argv)
{
    /* Create some parsers */
    mpc_parser_t *number = mpc_new("number");
    mpc_parser_t *operator = mpc_new("operator");
    mpc_parser_t *expr = mpc_new("expr");
    mpc_parser_t *byol = mpc_new("byol");

    /* Define them with the following language */
    mpca_lang(MPCA_LANG_DEFAULT,
              "number   : /-?[0-9]+(\\.[0-9])?/ ;"
              "operator : '+' | '-' | '*' | '/' | '%' | \"add\" | \"sub\" | \"mul\" | \"div\" ;"
              "expr     : <number> | '(' <operator> <expr>+ ')' ;"
              "byol     : /^/ <operator> <expr>+ /$/ ;",
              number, operator, expr, byol);


    puts("Byol version 0.0.2");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
        char *input = readline("Byol> ");
        add_history(input);

        /* Attempt to parse the user input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, byol, &r)) {
            /* On success print and delete the AST */
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            /* Otherwise, print and delete the error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        free(input);
    }

    /* Undefine and delete our parsers */
    mpc_cleanup(4, number, operator, expr, byol);
    
    return 0;
}
