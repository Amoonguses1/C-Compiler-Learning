#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// the types of abstruct syntax tree
typedef enum
{
    ND_ADD, // "+"
    ND_SUB, // "-"
    ND_MUL, // "*"
    ND_DIV, // "/"
    ND_NUM, // integer
} NodeKind;

typedef struct Node Node;

// the type of the abstract syntax tree
struct Node
{
    NodeKind kind; // the types of nodes
    Node *lhs;     // left-hand side
    Node *rhs;     // right-hand side
    int val;       // use this components if kind == ND_NUM
};

// prototype
Node *primary();
Node *mul();
Node *expr();

bool consume(char op);
int expect_number();
void expect(char op);

// generates new node which express binary operator.
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// generates new node which express a number.
Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// processes the following matching generation rule.
//
// mul = primary ( "*" primary | "/" primary) *
Node *mul()
{
    Node *node = primary();

    for (;;)
    {
        if (consume('*'))
            node = new_node(ND_MUL, node, primary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, primary());
        else
            return node;
    }
}

// processes the following matching generation rule.
//
// primary = "(" expr ")" | num
Node *primary()
{
    if (consume('('))
    {
        Node *node = expr();
        expect(')');
        return node;
    }

    return new_node_num(expect_number());
}

// processes the following matching generation rule.
//
// expr = mul ("+" mul | "-" mul) *
Node *expr()
{
    Node *node = mul();

    for (;;)
    {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

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

// input program
char *user_input;

// report where error occuers
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
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
        error_at(token->str, "expected '%c'", op);
    }
    token = token->next;
}

// if the current token is a number,
// return the value and step forward.
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "expected a number");
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

// tokenize the input string 'input char' and return the start token
Token *tokenize()
{
    Token head;
    head.next = NULL;
    Token *cur = &head;
    char *p = user_input;
    while (*p)
    {
        // skip the space character
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/')
        {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (*p == '(' || *p == ')')
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
    }

    printf("    str x0, [sp, 0]\n");
    printf("    add sp, sp, 16\n");
}
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
    Node *node = expr();

    // assembler start
    printf(".globl main\n");
    printf("main:\n");

    // code generation walking the AST.
    gen(node);

    // pop the stack top and treat as return value
    printf("    sub sp, sp, 16\n");
    printf("    ldr x0, [sp, 0]\n");
    printf("    ret\n");
    return 0;
}
