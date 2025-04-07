#include "9cc.h"

Program *program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Var *locals;

// Find a local variable by name.
Var *find_var(Token *tok)
{
    for (Var *var = locals; var; var = var->next)
        if (strlen(var->name) == tok->len && !memcmp(tok->str, var->name, tok->len))
            return var;
    return NULL;
}

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

Node *new_var(Var *var)
{
    Node *node = new_node(ND_VAR);
    node->var = var;
    return node;
}

Var *push_var(char *name)
{
    Var *var = calloc(1, sizeof(Var));
    var->next = locals;
    var->name = name;
    locals = var;
    return var;
}

Node *read_expr_stmt()
{
    return new_node_unary(ND_EXPR_STMT, expr());
}

// processes the following matching generation rule.
//
// program = stmt*
Program *program()
{
    locals = NULL;

    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!at_eof())
    {
        cur->next = stmt();
        cur = cur->next;
    }

    Program *prog = calloc(1, sizeof(Program));
    prog->node = head.next;
    prog->locals = locals;
    return prog;
}

// processes the following matching generation rule.
//
// stmt =
//      "return" expr ";"
//      | expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? "; expr? ";" expr? ")" stmt
Node *stmt()
{
    if (consume("return"))
    {
        Node *node = new_node_unary(ND_RETURN, expr());
        expect(";");
        return node;
    }

    if (consume("if"))
    {
        Node *node = new_node(ND_IF);
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

    if (consume("while"))
    {
        Node *node = new_node(ND_WHILE);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }

    if (consume("for"))
    {
        Node *node = new_node(ND_FOR);
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
        Var *var = find_var(tok);
        if (!var)
        {
            var = push_var(strndup(tok->str, tok->len));
        }
        return new_var(var);
    }

    return new_node_num(expect_number());
}
