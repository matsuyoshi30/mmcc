#include "mmcc.h"

Type *int_type = &(Type){TY_INT, 4};

// Parser

Type *pointer_to(Type *ty) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_PTR;
    type->size = 8;
    type->ptr_to = ty;
    return type;
}

void check_type(Node *node) {
    if (!node || node->type)
        return;

    check_type(node->lhs);
    check_type(node->rhs);

    for (Node *n=node->blocks; n; n=n->next)
        check_type(n);
    for (Node *n=node->args; n; n=n->next)
        check_type(n);

    switch (node->kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
    case ND_AS:
        node->type = node->lhs->type;
        return;
    case ND_LV:
        node->type = node->lvar->type;
        return;
    case ND_ADDR:
        if (node->lhs->type->kind == TY_ARR)
            node->type = pointer_to(node->lhs->type->ptr_to);
        else
            node->type = pointer_to(node->lhs->type);
        return;
    case ND_DEREF:
        if (!node->lhs->type->ptr_to)
            error("invalid pointer dereference");
        node->type = node->lhs->type->ptr_to;
        return;
    case ND_FUNC:
    case ND_NUM:
        node->type = int_type;
        return;
    }
}

Type *array_of(Type *ty, int n) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_ARR;
    type->size = ty->size * n;
    type->ptr_to = ty;
    type->size_array = n;
    return type;
}

Type *read_type_suffix(Type *ty) {
    if (!consume("["))
        return ty;
    int num = expect_number();
    expect("]");
    return array_of(ty, num);
}

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

Node *new_add(Node *lhs, Node *rhs) {
    check_type(lhs);
    check_type(rhs);

    if (lhs->type->kind == TY_INT && rhs->type->kind == TY_INT)
        return new_node(ND_ADD, lhs, rhs);
    else if ((lhs->kind == ND_ADDR) || (lhs->type->kind == TY_ARR) || (lhs->type->kind == TY_PTR))
        rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size));

    return new_node(ND_ADD, lhs, rhs);
}

Node *new_sub(Node *lhs, Node *rhs) {
    check_type(lhs);
    check_type(rhs);

    if (lhs->type->kind == TY_INT && rhs->type->kind == TY_INT)
        return new_node(ND_SUB, lhs, rhs);
    else if ((lhs->kind == ND_ADDR) || (lhs->type->kind == TY_ARR) || (lhs->type->kind == TY_PTR))
        rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size));

    return new_node(ND_SUB, lhs, rhs);
}

LVar *locals;

LVar *find_lvar(Token *tok) {
    for (LVar *lvar = locals; lvar; lvar=lvar->next)
        if (strlen(lvar->name) == tok->len && !strncmp(tok->str, lvar->name, tok->len))
            return lvar;
    return NULL;
}

Function *code;

void program();
Function *function();
LVar *funcparams();
Node *stmt();
Node *declaration();
Type *basetype();
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

// function = basetype ident "(" funcparams* ")" "{" stmt* "}"
Function *function() {
    Function *func = calloc(1, sizeof(Function));
    Type *ty = basetype();
    char *funcname = expect_ident();

    expect("(");

    locals = calloc(1, sizeof(LVar));
    locals->type = ty;
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
        check_type(cur->next);
        cur = cur->next;
    }

    func->type = ty;
    func->name = funcname;
    func->params = params;
    func->locals = locals;
    func->body = head.next;

    return func;
}

// funcparams = ( basetype ident ( "," basetype ident )* )?
LVar *funcparams() {
    if (consume(")"))
        return NULL; // no parameters

    Type *ty = basetype();
    char *name = expect_ident();

    LVar *params = calloc(1, sizeof(LVar));
    params->type = ty;
    params->name = name;
    params->offset = ty->size;
    LVar *cur = params;

    while (consume(",")) {
        ty = basetype();
        name = expect_ident();
        cur->next = calloc(1, sizeof(LVar));
        cur->next->type = ty;
        cur->next->name = name;
        cur->next->offset = cur->offset + ty->size;
        cur = cur->next;
    }
    expect(")");

    return params;
}

// stmt = "return" expr ";"
//        | "{" stmt* "}"
//        | "if" "(" cond ")" stmt ( "else" stmt )?
//        | "while" "(" cond ")" stmt
//        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//        | declaration ";"
//        | expr ";"
Node *stmt() {
    Node *node;

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
            check_type(cur->next);
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
        check_type(node->cond);
        expect(")");
        node->then = stmt();
        if (consume_tk(TK_ELSE)) {
            node->els = stmt();
            check_type(node->els);
        }
        return node;
    }

    if (consume_tk(TK_WHILE)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = cond();
        check_type(node->cond);
        expect(")");
        node->then = stmt();
        check_type(node->then);
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
            check_type(node->preop);
            expect(";");
        }

        if (consume(";")) {
            node->cond = NULL;
        } else {
            node->cond = expr();
            check_type(node->cond);
            expect(";");
        }

        if (consume(")")) {
            node->postop = NULL;
        } else {
            node->postop = expr();
            check_type(node->postop);
            expect(")");
        }

        node->then = stmt();
        check_type(node->then);
        return node;
    }

    if (peek("int")) {
        node = declaration();
        check_type(node);
        expect(";");
        return node;
    }

    node = expr();
    expect(";");

    return node;
}

// declaration = basetype ident (( "[" num "]" ) | ( "=" expr ))?
Node *declaration() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LV;

    Type *ty = basetype();
    char *ident = expect_ident();
    ty = read_type_suffix(ty);

    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->type = ty;
    lvar->next = locals;
    lvar->name = ident;
    lvar->offset = locals->offset + ty->size;
    locals = lvar;

    node->lvar = lvar;

    if (consume("="))
        node = new_node(ND_AS, node, expr());

    return node;
}

// basetype = type ( "*" )*
Type *basetype() {
    expect_type();
    Type *type = int_type;

    while (consume("*"))
        type = pointer_to(type);
    return type;
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
            node = new_add(node, mul());
        else if (consume("-"))
            node = new_sub(node, mul());
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

// unary = ( '+' | '-' )? primary | '&' unary | '*' unary | "sizeof" unary
Node *unary() {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());
    if (consume("&"))
        return new_node_addr(unary());
    if (consume("*"))
        return new_node_deref(unary());

    if (consume_tk(TK_SIZEOF)) {
        Node *node = unary();
        check_type(node);
        if (node->type->kind == TY_INT)
            return new_node_num(4);
        else if (node->type->kind == TY_PTR)
            return new_node_num(8);
    }

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
