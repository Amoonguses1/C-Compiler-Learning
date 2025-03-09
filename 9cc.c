#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Token Types
typedef enum
{
    TK_RESERVED, // symbols
    TK_NUM,      // integer token
    TK_EOF,      // end of input
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind; // token types
    Token *next;
    int val;   // if kind == TK_NUM, this field represents the integer
    char *str; // token string
};

// current target token
Token *token;

// error report function
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// if the next token is the specified symbol,
// step forward and return true
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
    {
        return false;
    }

    token = token->next;
    return true;
}

// if the next token is the expected symbol,
// step forward.
// if not, report error
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
    {
        error("expected %c, but got: ", op, token->str[0]);
    }
    token = token->next;
}

// if the current token is a number,
// return the value and step forward.
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error("the current token is not a number");
    }

    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// create a new token and set it as the next token after the current token.
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// tokenize the input string 'p' and return the start token
Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p)
    {
        // skip the space character
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("failed to tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "invalid args\n");
        return 1;
    }

    // tokenize
    token = tokenize(argv[1]);

    // assembler start
    printf(".globl main\n");
    printf("main:\n");

    // the start of the expression must be a number,
    // check the top token and output mov instruction.
    printf("    mov x0, %d\n", expect_number());

    // consume the consecutive tokens such as `+ <number>` or `- <number>`
    // and output the assembler
    while (!at_eof())
    {
        if (consume('+'))
        {
            printf("    add x0, x0, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub x0, x0, %d\n", expect_number());
    }

    printf("    ret \n");
    return 0;
}
