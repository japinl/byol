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

/*
 * Create enumeration of possible error types.
 */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/*
 * Create enumeration of possible lval types.
 */
enum { LVAL_NUM, LVAL_ERR };

/*
 * Declare new lval structure.
 */
typedef struct {
    int type;
    union {
        long num;
        int err;
    } data;
} lval;

/*
 * Create a new number type lval.
 */
lval lval_num(long x)
{
    lval v;
    v.type = LVAL_NUM;
    v.data.num = x;
    return v;
}

/*
 * Create a new error type lval.
 */
lval lval_err(int x)
{
    lval v;
    v.type = LVAL_ERR;
    v.data.err = x;
    return v;
}

/*
 * Print an "lval".
 */
void lval_print(lval v)
{
    switch (v.type) {
        /*
         * In the case the type is a number print it.
         * Then 'break' out of the switch.
         */
    case LVAL_NUM:
        printf("%li", v.data.num);
        break;

        /*
         * In the case the type is an error.
         */
    case LVAL_ERR:
        if (v.data.err == LERR_DIV_ZERO) {
            printf("Error: Division by zero!");
        }
        if (v.data.err == LERR_BAD_OP) {
            printf("Error: Invalid operator!");
        }
        if (v.data.err == LERR_BAD_NUM) {
            printf("Error: Invalid number!");
        }
        break;
    }
}

/*
 * Print an "lval" followed by a newline.
 */
void lval_println(lval v)
{
    lval_print(v);
    putchar('\n');
}


/*
 * @brief Use operator string to see which operation to preform.
 */
lval eval_op(lval x, char *op, lval y)
{
    /* If either value is an error return it. */
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    /* Otherwise do maths on the number values */
    if (strcmp(op, "+") == 0) { return lval_num(x.data.num + y.data.num); }
    if (strcmp(op, "-") == 0) { return lval_num(x.data.num - y.data.num); }
    if (strcmp(op, "*") == 0) { return lval_num(x.data.num * y.data.num); }
    if (strcmp(op, "/") == 0) {
        return y.data.num == 0 ?
            lval_err(LERR_DIV_ZERO) :
            lval_num(x.data.num / y.data.num);
    }
    if (strcmp(op, "%") == 0) {
        return y.data.num == 0 ?
            lval_err(LERR_DIV_ZERO) :
            lval_num(x.data.num % y.data.num);
    }
    if (strcmp(op, "^") == 0) { return lval_num(x.data.num << y.data.num); }
    if (strcmp(op, "min") == 0) {
        return lval_num(x.data.num < y.data.num ? x.data.num : y.data.num);
    }
    if (strcmp(op, "max") == 0) {
        return lval_num(x.data.num < y.data.num ? y.data.num : x.data.num);
    }

    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t)
{
    /* If tagged as number return directly. */
    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    /* The operator is always stored in second child. */
    char *op = t->children[1]->contents;

    /* Store the third child in `x'. */
    lval x = eval(t->children[2]);

    /* Iterate the remaining children and combining. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

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
              "operator : '+' | '-' | '*' | '/' | '%' | '^' | \"add\" | \"sub\" | \"mul\" | \"div\" | \"min\" | \"max\" ;"
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
            lval result = eval(r.output);
            lval_println(result);
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
