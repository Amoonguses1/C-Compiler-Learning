#include "9cc.h"

Function *program();
Function *function();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

VarList *locals;

// Find a local variable by name.
Var *find_var(Token *tok)
{
    for (VarList *vl = locals; vl; vl = vl->next)
    {
        Var *var = vl->var;
        if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
        {
            return var;
        }
    }
    return NULL;
}

// generates new node which express binary operator.
Node *new_node(NodeKind kind, Token *tok)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->tok = tok;
    return node;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
    Node *node = new_node(kind, tok);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_unary(NodeKind kind, Node *expr, Token *tok)
{
    Node *node = new_node(kind, tok);
    node->lhs = expr;
    return node;
}

// generates new node which express a number.
Node *new_node_num(int val, Token *tok)
{
    Node *node = new_node(ND_NUM, tok);
    node->val = val;
    return node;
}

Node *new_var(Var *var, Token *tok)
{
    Node *node = new_node(ND_VAR, tok);
    node->var = var;
    return node;
}

Var *push_var(char *name)
{
    Var *var = calloc(1, sizeof(Var));
    var->name = name;

    VarList *vl = calloc(1, sizeof(VarList));
    vl->var = var;
    vl->next = locals;
    locals = vl;
    return var;
}

Node *read_expr_stmt()
{
    Token *tok = token;
    return new_node_unary(ND_EXPR_STMT, expr(), tok);
}

// processes the following matching generation rule.
//
// program = function*
Function *program()
{
    Function head;
    head.next = NULL;
    Function *cur = &head;

    while (!at_eof())
    {
        cur->next = function();
        cur = cur->next;
    }
    return head.next;
}

VarList *read_func_params()
{
    if (consume(")"))
    {
        return NULL;
    }

    VarList *head = calloc(1, sizeof(VarList));
    head->var = push_var(expect_ident());
    VarList *cur = head;

    while (!consume(")"))
    {
        expect(",");
        cur->next = calloc(1, sizeof(VarList));
        cur->next->var = push_var(expect_ident());
        cur = cur->next;
    }

    return head;
}

// processes the following matching generation rule.
//
// function = ident "(" params? ")" "{" stmt* "}"
// params = idenx ( "," ident)*
Function *function()
{
    locals = NULL;

    Function *fn = calloc(1, sizeof(Function));
    fn->name = expect_ident();
    expect("(");
    fn->params = read_func_params();
    expect("{");

    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!consume("}"))
    {
        cur->next = stmt();
        cur = cur->next;
    }

    fn->node = head.next;
    fn->locals = locals;
    return fn;
}

// processes the following matching generation rule.
//
// stmt =
//      "return" expr ";"
//      | "{" stmt* "}"
//      | expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? "; expr? ";" expr? ")" stmt
Node *stmt()
{
    Token *tok;
    if (tok = consume("return"))
    {
        Node *node = new_node_unary(ND_RETURN, expr(), tok);
        expect(";");
        return node;
    }
    if (tok = consume("{"))
    {
        Node head;
        head.next = NULL;
        Node *cur = &head;

        while (!consume("}"))
        {
            cur->next = stmt();
            cur = cur->next;
        }
        Node *node = new_node(ND_BLOCK, tok);
        node->body = head.next;
        return node;
    }

    if (tok = consume("if"))
    {
        Node *node = new_node(ND_IF, tok);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else"))
        {
            node->els = stmt();
        }
        return node;
    }

    if (tok = consume("while"))
    {
        Node *node = new_node(ND_WHILE, tok);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }

    if (tok = consume("for"))
    {
        Node *node = new_node(ND_FOR, tok);
        expect("(");
        if (!consume(";"))
        {
            node->init = read_expr_stmt();
            expect(";");
        }
        if (!consume(";"))
        {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")"))
        {
            node->inc = read_expr_stmt();
            expect(")");
        }
        node->then = stmt();
        return node;
    }

    Node *node = read_expr_stmt();
    expect(";");
    return node;
}

// processes the following matching generation rule.
//
// expr = assign
Node *expr()
{
    return assign();
}

// processes the following matching generation rule.
//
// assign = equality ( "=" assign)?
Node *assign()
{
    Node *node = equality();
    Token *tok;
    if (tok = consume("="))
    {
        node = new_node_binary(ND_ASSIGN, node, assign(), tok);
    }
    return node;
}

// processes the following matching generation rule.
//
// euqality = relational ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();
    Token *tok;
    for (;;)
    {
        if (tok = consume("=="))
            node = new_node_binary(ND_EQ, node, relational(), tok);
        else if (tok = consume("!="))
            node = new_node_binary(ND_NE, node, relational(), tok);
        else
            return node;
    }
}

// processes the following matching generation rule.
//
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
    Node *node = add();
    Token *tok;

    for (;;)
    {
        if (tok = consume("<="))
            node = new_node_binary(ND_LE, node, add(), tok);
        else if (tok = consume("<"))
            node = new_node_binary(ND_LT, node, add(), tok);
        else if (tok = consume(">="))
            node = new_node_binary(ND_LE, add(), node, tok);
        else if (tok = consume(">"))
            node = new_node_binary(ND_LT, add(), node, tok);
        else
            return node;
    }
}

// processes the following matching generation rule.
//
// add = mul ("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();
    Token *tok;

    for (;;)
    {
        if (consume("+"))
            node = new_node_binary(ND_ADD, node, mul(), tok);
        else if (consume("-"))
            node = new_node_binary(ND_SUB, node, mul(), tok);
        else
            return node;
    }
}

// processes the following matching generation rule.
//
// mul = unary ("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();
    Token *tok;

    for (;;)
    {
        if (consume("*"))
            node = new_node_binary(ND_MUL, node, unary(), tok);
        else if (consume("/"))
            node = new_node_binary(ND_DIV, node, unary(), tok);
        else
            return node;
    }
}

// processes the following matching generation rule.
//
// unary = ("+" | "-")? primary | "*" unary | "&" unary
Node *unary()
{
    Token *tok;

    if (consume("+"))
        return unary();
    if (tok = consume("-"))
        return new_node_binary(ND_SUB, new_node_num(0, tok), unary(), tok);
    if (tok = consume("&"))
        return new_node_unary(ND_ADDR, unary(), tok);
    if (tok = consume("*"))
        return new_node_unary(ND_DEREF, unary(), tok);

    return primary();
}

// processes the following matching generation rule.
//
// func-args = "(" (assign ("," assign)*)? ")"
Node *func_args()
{
    if (consume(")"))
    {
        return NULL;
    }

    Node *head = assign();
    Node *cur = head;
    while (consume(","))
    {
        cur->next = assign();
        cur = cur->next;
    }
    expect(")");
    return head;
}

// processes the following matching generation rule.
//
// primary = "(" expr ")" | num | ident func-args?
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok;
    if (tok = consume_ident())
    {
        if (consume("("))
        {
            Node *node = new_node(ND_FUNCALL, tok);
            node->funcname = strndup(tok->str, tok->len);
            node->args = func_args();
            return node;
        }

        Var *var = find_var(tok);
        if (!var)
        {
            var = push_var(strndup(tok->str, tok->len));
        }
        return new_var(var, tok);
    }

    return new_node_num(expect_number(), tok);
}
