#include "9cc.h"

Node *program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// generates new node which express binary operator.
Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_unary(NodeKind kind, Node *expr)
{
    Node *node = new_node(kind);
    node->lhs = expr;
    return node;
}

// generates new node which express a number.
Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

Node *new_lvar(char name)
{
    Node *node = new_node(ND_LVAR);
    node->name = name;
    return node;
}

// processes the following matching generation rule.
//
// program = stmt*
Node *program()
{
    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!at_eof())
    {
        cur->next = stmt();
        cur = cur->next;
    }
    return head.next;
}

// processes the following matching generation rule.
//
// stmt = "return" expr ";" | expr ";"
Node *stmt()
{
    if (consume("return"))
    {
        Node *node = new_node_unary(ND_RETURN, expr());
        expect(";");
        return node;
    }

    Node *node = new_node_unary(ND_EXPR_STMT, expr());
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
    if (consume("="))
    {
        node = new_node_binary(ND_ASSIGN, node, assign());
    }
    return node;
}

// processes the following matching generation rule.
//
// euqality = relational ("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
            node = new_node_binary(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node_binary(ND_NE, node, relational());
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

    for (;;)
    {
        if (consume("<="))
            node = new_node_binary(ND_LE, node, add());
        else if (consume("<"))
            node = new_node_binary(ND_LT, node, add());
        else if (consume(">="))
            node = new_node_binary(ND_LE, add(), node);
        else if (consume(">"))
            node = new_node_binary(ND_LT, add(), node);
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

    for (;;)
    {
        if (consume("+"))
            node = new_node_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node_binary(ND_SUB, node, mul());
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

    for (;;)
    {
        if (consume("*"))
            node = new_node_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

// processes the following matching generation rule.
//
// unary = ("+" | "-")? primary
Node *unary()
{
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node_binary(ND_SUB, new_node_num(0), unary());

    return primary();
}

// processes the following matching generation rule.
//
// primary = "(" expr ")" | num | ident
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok)
    {
        return new_lvar(*tok->str);
    }

    return new_node_num(expect_number());
}
