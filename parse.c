#include "mmcc.h"

// Parser

Node *new_node(Nodekind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_addr(Node *target) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_ADDR;
    node->lhs = target;
    return node;
}

Node *new_node_deref(Node *target) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEREF;
    node->lhs = target;
    return node;
}

Node *new_node_func(char *funcname, Node *args) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    node->funcname = funcname;
    node->args = args;
    return node;
}

Node *new_node_lvar(LVar *lvar) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LV;
    node->lvar = lvar;
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

// function = type ident "(" funcparams* ")" "{" stmt* "}"
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

    func->type = ty;
    func->name = funcname;
    func->params = params;
    func->locals = locals;
    func->body = head.next;

    return func;
}

// funcparams = ( type ident ( "," type ident )* )?
LVar *funcparams() {
    if (consume(")"))
        return NULL; // no parameters

    Type *ty = consume_type();
    Token *tok = consume_ident();

    LVar *params = calloc(1, sizeof(LVar));
    params->type = ty;
    params->name = strndup(tok->str, tok->len);
    params->len = tok->len;
    params->offset = 8;
    LVar *cur = params;

    while (consume(",")) {
        ty = consume_type();
        tok = consume_ident();
        cur->next = calloc(1, sizeof(LVar));
        cur->next->type = ty;
        cur->next->name = strndup(tok->str, tok->len);
        cur->next->len = tok->len;
        cur->next->offset = cur->offset + 8;
        cur = cur->next;
    }
    expect(")");

    return params;
}

// stmt = type ( "*" )* ident ( "=" expr )? ";"
//        | "return" expr ";"
//        | "{" stmt* "}"
//        | "if" "(" cond ")" stmt ( "else" stmt )?
//        | "while" "(" cond ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | expr ";"
Node *stmt() {
    Node *node;

    Type *ty = consume_type();
    if (ty) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_LV;

        Token *tok = consume_ident();

        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->type = ty;
        lvar->next = locals;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->offset = locals->offset + 8;
        node->lvar = lvar;
        locals = node->lvar;

        if (consume("="))
            node = new_node(ND_AS, node, expr());

        expect(";");

        return node;
    }

    if (consume_tk(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RET;
        node->lhs = expr();
        expect(";");
        return node;
    }

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

    if (consume_tk(TK_IF)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = cond();
        expect(")");
        node->then = stmt();
        if (consume_tk(TK_ELSE))
            node->els = stmt();
        return node;
    }

    if (consume_tk(TK_WHILE)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = cond();
        expect(")");
        node->then = stmt();
        return node;
    }

    if (consume_tk(TK_FOR)) {
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
    }

    node = expr();
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

// unary = ( '+' | '-' )? primary | '&' unary | '*' unary
Node *unary() {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());
    if (consume("&"))
        return new_node_addr(unary());
    if (consume("*"))
        return new_node_deref(unary());

    return primary();
}

// primary = '(' expr ')' | ident ( "(" ( funcargs )* ")" )? | num
Node *primary() {
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        if (consume("("))
            return new_node_func(strndup(tok->str, tok->len), funcargs());
        else
            return new_node_lvar(find_lvar(tok));
    }

    return new_node_num(expect_number());
}

// funcargs = ( add ( "," add )* )?
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
