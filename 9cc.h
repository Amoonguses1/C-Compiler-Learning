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
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

extern char *user_input;
extern Token *token;

//
// parse.c
//

// the types of abstruct syntax tree
typedef enum
{
    ND_ADD, // "+"
    ND_SUB, // "-"
    ND_MUL, // "*"
    ND_DIV, // "/"
    ND_NUM, // integer
    ND_EQ,  // "=="
    ND_NE,  // "!="
    ND_LT,  // "<"
    ND_LE,  // "<="
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

Node *expr();

//
// codegen.c
//

void codegen(Node *node);
