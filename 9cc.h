#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//

// Token Types
typedef enum
{
    TK_RESERVED, // symbols
    TK_IDENT,    // identifier
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
    int len;   // the length of token
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
char *strndup(char *p, int len);
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
char *expect_ident();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *user_input;
extern Token *token;

//
// parse.c
//

typedef struct Var Var;
struct Var
{
    Var *next;
    char *name; // Variable name
    int offset; // Offset from RBP
};

// the types of abstruct syntax tree
typedef enum
{
    ND_ADD,       // "+"
    ND_SUB,       // "-"
    ND_MUL,       // "*"
    ND_DIV,       // "/"
    ND_ASSIGN,    // =
    ND_NUM,       // integer
    ND_RETURN,    // "return"
    ND_IF,        // "if"
    ND_WHILE,     // "while"
    ND_FOR,       // "for"
    ND_BLOCK,     // { ... } compound statement
    ND_FUNCALL,   // Function call
    ND_EXPR_STMT, // Expression statement
    ND_VAR,       // variable
    ND_EQ,        // "=="
    ND_NE,        // "!="
    ND_LT,        // "<"
    ND_LE,        // "<="
} NodeKind;

typedef struct Node Node;

// the type of the abstract syntax tree
struct Node
{
    NodeKind kind; // the types of nodes
    Node *next;    // next node

    Node *lhs; // left-hand side
    Node *rhs; // right-hand side

    // if, while, or for statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // compound statement
    Node *body;

    // Function call
    char *funcname;
    Node *args;

    int val;  // use this components if kind == ND_NUM
    Var *var; // use this components if kind == ND_VAR
};

typedef struct Function Function;
struct Function
{
    Function *next;
    char *name;
    Node *node;
    Var *locals;
    int stack_size;
};

Function *program();

//
// codegen.c
//

void codegen(Function *prog);
