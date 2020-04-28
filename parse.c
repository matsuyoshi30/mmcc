#include "mmcc.h"

// Parser

Node *new_node(Nodekind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

LVar *locals;

LVar *find_lvar(Token *tok) {
    for (LVar *lvar = locals; lvar; lvar=lvar->next)
        if (lvar->len == tok->len && !strncmp(tok->str, lvar->name, lvar->len))
            return lvar;
    return NULL;
}

Function *code;

void program();
Function *function();
LVar *funcparams();
Node *stmt();
Node *cond();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *funcargs();

// program = function*
void program() {
    Function head;
    head.next = NULL;
    Function *cur = &head;

    while (!at_eof()) {
        cur->next = function();
        cur = cur->next;
    }

    code = head.next;
}

// function = type ident "(" (type params)* ")" "{" stmt* "}"
Function *function() {
    Function *func = calloc(1, sizeof(Function));
    Type *ty = consume_type();
    char *funcname = expect_ident();

    expect("(");

    locals = calloc(1, sizeof(LVar));
    locals->offset = 0;

    LVar *params = funcparams();
    if (params)
        locals = params;

    expect("{");

    Node head;
    head.next = NULL;
    Node *cur = &head;

    while (!consume("}")) {
        cur->next = stmt();
        cur = cur->next;
    }

    func->type = ty->kind;
    func->name = funcname;
    func->params = params;
    func->locals = locals;
    func->body = head.next;

    return func;
}

LVar *funcparams() {
    if (consume(")"))
        return NULL; // no parameters

    LVar params;
    params.next = NULL;
    LVar *cur = &params;

    Type *ty = consume_type();
    Token *tok = consume_ident();
    while (ty && tok) {
        cur->next = calloc(1, sizeof(LVar));
        cur->next->type = ty->kind;
        cur->next->name = strndup(tok->str, tok->len);
        cur->next->len = tok->len;
        cur->next->offset = cur->offset + 8;
        cur = cur->next;

        if (!consume(","))
            break;
        ty = consume_type();
        tok = consume_ident();
    }
    expect(")");

    return params.next;
}

// stmt = "return" expr ";"
//        | type ident ";"
//        | "{" stmt* "}"
//        | "if" "(" cond ")" stmt ( "else" stmt )?
//        | "while" "(" cond ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | expr ";"
Node *stmt() {
    Node *node;

    if (consume("{")) {
        Node head;
        head.next = NULL;
        Node *cur = &head;

        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        while (!consume("}")) {
            cur->next = stmt();
            cur = cur->next;
        }
        node->blocks = head.next;
        return node;
    }

    Type *ty = consume_type();
    if (ty) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_LV;

        Token *tok = consume_ident();

        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = locals->offset + 8;
        node->offset = lvar->offset;
        locals = lvar;

        expect(";");

        return node;
    }

    if (consume_tk(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RET;
        node->lhs = expr();
    } else if (consume_tk(TK_IF)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = cond();
        expect(")");
        node->then = stmt();
        if (consume_tk(TK_ELSE))
            node->els = stmt();
        return node;
    } else if (consume_tk(TK_WHILE)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = cond();
        expect(")");
        node->then = stmt();
        return node;
    } else if (consume_tk(TK_FOR)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");

        if (consume(";")) {
            node->preop = NULL;
        } else {
            node->preop = expr();
            expect(";");
        }

        if (consume(";")) {
            node->cond = NULL;
        } else {
            node->cond = expr();
            expect(";");
        }

        if (consume(")")) {
            node->postop = NULL;
        } else {
            node->postop = expr();
            expect(")");
        }

        node->then = stmt();
        return node;
    } else {
        node = expr();
    }

    expect(";");

    return node;
}

// cond = expr
Node *cond() {
    return expr();
}

// expr = assign
Node *expr() {
    return assign();
}

// assign = equality ( "=" assign )?
Node *assign() {
    Node *node = equality();

    if (consume("="))
        node = new_node(ND_AS, node, assign());

    return node;
}

// equality = relational ( "==" relational | "!=" relational )*
Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// relational = add ( ">" add | "<" add | ">=" add | "<=" add )*
Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume(">="))
            node = new_node(ND_OM, node, add());
        else if (consume("<="))
            node = new_node(ND_OL, node, add());
        else if (consume(">"))
            node = new_node(ND_MT, node, add());
        else if (consume("<"))
            node = new_node(ND_LT, node, add());
        else
            return node;
    }
}

// add = mul ( "+" mul | "-" mul )*
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// mul = unary ( '*' unary | '/' unary )*
Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// unary = ( '+' | '-' )? primary
Node *unary() {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());

    return primary();
}

// primary = '(' expr ')' | ident ( "(" (args)* ")" )? | num
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        if (consume("(")) {
            node->kind = ND_FUNC;
            node->funcname = strndup(tok->str, tok->len);
            node->args = funcargs();
        } else {
            node->kind = ND_LV;

            LVar *lvar = find_lvar(tok);
            node->offset = lvar->offset;
        }

        return node;
    }

    return new_node_num(expect_number());
}

Node *funcargs() {
    if (consume(")"))
        return NULL; // no argument

    Node *head = add();
    Node *cur = head;
    while (consume(",")) {
        cur->next = add();
        cur = cur->next;
    }
    expect(")");

    return head;
}
