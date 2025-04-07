#include "9cc.h"

char *user_input;
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

char *strndup(char *p, int len)
{
    char *buf = malloc(len + 1);
    strncpy(buf, p, len);
    buf[len] = '\0';
    return buf;
}

// if the next token is the specified symbol,
// step forward and return true
bool consume(char *op)
{
    if (token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        strncmp(token->str, op, token->len))
    {
        return false;
    }

    token = token->next;
    return true;
}

// if the next token is a local variable,
// step forward and return the token
Token *consume_ident()
{
    if (token->kind != TK_IDENT)
    {
        return NULL;
    }

    Token *t = token;
    token = token->next;
    return t;
}

// if the next token is the expected symbol,
// step forward.
// if not, report error
void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len || strncmp(token->str, op, strlen(op)))
    {
        error_at(token->str, "expected '%s'", op);
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

bool starts_with(char *str, char *op)
{
    return strncmp(str, op, strlen(op)) == 0;
}

// create a new token and set it as the next token after the current token.
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool is_alpha(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

bool is_alnum(char c)
{
    return is_alpha(c) || ('0' <= c && c <= '9');
}

char *starts_withreserved(char *p)
{
    // Keywords
    static char *kw[] = {"return", "if", "else", "while", "for"};
    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    {
        int len = strlen(kw[i]);
        if (starts_with(p, kw[i]) && !is_alnum(p[len]))
        {
            return kw[i];
        }
    }

    // multi-letter
    static char *ops[] = {"==", "!=", "<=", ">="};
    for (int i = 0; i < sizeof(ops) / sizeof(*ops); i++)
    {
        if (starts_with(p, ops[i]))
        {
            return ops[i];
        }
    }

    return NULL;
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

        // Keyword or multi-letter punctuator
        char *kw = starts_withreserved(p);
        if (kw)
        {
            int len = strlen(kw);
            cur = new_token(TK_RESERVED, cur, p, len);
            p += len;
            continue;
        }

        if (strchr("+-*/()<>;={}", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // Identifier
        if (is_alpha(*p))
        {
            char *q = p++;
            while (is_alnum(*p))
            {
                p++;
            }
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("failed to tokenize");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
