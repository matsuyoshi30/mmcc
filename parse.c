#include "mmcc.h"

Type *char_type = &(Type){TY_CHAR, 1};
Type *int_type = &(Type){TY_INT, 4};

static int lc = 0;

// Parser

bool is_integer(Type *type) {
    return type->kind == TY_INT || type->kind == TY_CHAR;
}

Type *pointer_to(Type *ty) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_PTR;
    type->size = 8;
    type->ptr_to = ty;
    return type;
}

Type *array_of(Type *ty, int n) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_ARR;
    type->size = ty->size * n;
    type->ptr_to = ty;
    type->size_array = n;
    return type;
}

void check_type(Node *node) {
    if (!node || node->type)
        return;

    check_type(node->lhs);
    check_type(node->rhs);
    check_type(node->cond);
    check_type(node->then);
    check_type(node->els);
    check_type(node->preop);
    check_type(node->postop);

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
        node->type = node->var->type;
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
    case ND_STMT_EXPR: {
        Node *block = node->blocks;
        while (block->next)
            block = block->next;
        node->type = block->type;
        return;
    }
    case ND_FUNC:
    case ND_NUM:
        node->type = int_type;
        return;
    }
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

Node *new_node_var(Var *var) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LV;
    node->var = var;
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

    // num + num
    if (is_integer(lhs->type) && is_integer(rhs->type))
        return new_node(ND_ADD, lhs, rhs);
    // ptr + ptr
    if (lhs->type->ptr_to && rhs->type->ptr_to)
        error("invalid operands");
    // num + ptr -> ptr + num
    if (rhs->type->ptr_to) {
        Node *temp = lhs;
        lhs = rhs;
        rhs = temp;
    }
    // num + ptr
    rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size));

    return new_node(ND_ADD, lhs, rhs);
}

Node *new_sub(Node *lhs, Node *rhs) {
    check_type(lhs);
    check_type(rhs);

    // num + num
    if (is_integer(lhs->type) && is_integer(rhs->type))
        return new_node(ND_SUB, lhs, rhs);
    // ptr + ptr
    if (lhs->type->ptr_to && rhs->type->ptr_to)
        error("invalid operands");
    // ptr + num
    rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size));

    return new_node(ND_SUB, lhs, rhs);
}

Var *locals;
Var *globals;

Var *new_lvar(Type *type, char *name) {
    Var *var = calloc(1, sizeof(Var));
    var->type = type;
    var->name = name;
    var->is_local = true;
    var->next = locals;
    locals = var;

    return var;
}

Var *new_gvar(Type *type, char *name) {
    Var *var = calloc(1, sizeof(Var));
    var->type = type;
    var->name = name;
    var->is_local = false;
    var->next = globals;
    globals = var;

    return var;
}

Var *find_var(Token *tok) {
    for (Var *lvar=locals; lvar; lvar=lvar->next)
        if (strlen(lvar->name) == tok->len && !strncmp(tok->str, lvar->name, tok->len))
            return lvar;

    for (Var *gvar=globals; gvar; gvar=gvar->next)
        if (strlen(gvar->name) == tok->len && !strncmp(tok->str, gvar->name, tok->len))
            return gvar;

    return NULL;
}

Var *strs;

Var *new_str(Token *token) {
    Var *string = calloc(1, sizeof(Var));
    string->type = array_of(char_type, token->strlen);
    string->str = strndup(token->str, token->len);
    string->next = strs;
    string->lc = lc++;
    strs = string;

    return string;
}

Function *code;

void program();
Function *function(Type *type, char *funcname);
Var *funcparams();
Node *stmt();
Node *declaration();
Type *basetype();
Type *type_suffix(Type *ty);
Node *expr_stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *postfix();
Node *primary();
Node *funcargs();
Node *read_array();

// program = ( basetype ident ( function | gvar ) )*
void program() {
    Function head;
    head.next = NULL;
    Function *cur = &head;

    strs = NULL;

    globals = calloc(1, sizeof(Node));

    while (!at_eof()) {
        Type *ty = basetype();
        char *name = expect_ident();

        if (peek("(")) {
            cur->next = function(ty, name);
            cur = cur->next;
        } else {
            ty = type_suffix(ty);
            new_gvar(ty, name);
            expect(";");
        }
    }

    code = head.next;
}

// function = "(" funcparams* ")" "{" stmt* "}"
Function *function(Type *type, char *funcname) {
    Function *func = calloc(1, sizeof(Function));

    expect("(");

    locals = funcparams();
    func->params = locals;

    expect("{");

    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume("}")) {
        cur->next = stmt();
        check_type(cur->next);
        cur = cur->next;
    }

    func->type = type;
    func->name = funcname;
    func->locals = locals;
    func->body = head.next;

    return func;
}

// funcparams = ( basetype ident type_suffix ( "," basetype ident type_suffix )* )?
Var *funcparams() {
    if (consume(")"))
        return NULL; // no parameters

    Type *ty = basetype();
    char *name = expect_ident();
    ty = type_suffix(ty);

    Var *params = new_lvar(ty, name);
    params->offset = ty->size;
    Var *cur = params;

    while (consume(",")) {
        ty = basetype();
        name = expect_ident();
        ty = type_suffix(ty);
        cur->next = calloc(1, sizeof(Var));
        cur->next->type = ty;
        cur->next->name = name;
        cur->next->offset = cur->offset + ty->size;
        cur->next->is_local = true;
        cur = cur->next;
    }
    expect(")");

    return params;
}

// stmt = "return" expr ";"
//        | "{" stmt* "}"
//        | "if" "(" expr ")" stmt ( "else" stmt )?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr_stmt? ";" expr? ";" expr_stmt? ")" stmt
//        | declaration ";"
//        | expr_stmt ";"
Node *stmt() {
    Node *node;

    if (consume("return")) {
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

    if (consume("if")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        check_type(node->cond);
        expect(")");
        node->then = stmt();
        if (consume("else")) {
            node->els = stmt();
            check_type(node->els);
        }
        return node;
    }

    if (consume("while")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        expect("(");
        node->cond = expr();
        check_type(node->cond);
        expect(")");
        node->then = stmt();
        check_type(node->then);
        return node;
    }

    if (consume("for")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");

        if (consume(";")) {
            node->preop = NULL;
        } else {
            if (peek("int") || peek("char"))
                node->preop = declaration();
            else
                node->preop = expr_stmt();
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
            node->postop = expr_stmt();
            check_type(node->postop);
            expect(")");
        }

        node->then = stmt();
        check_type(node->then);
        return node;
    }

    if (peek("int") || peek("char")) {
        node = declaration();
        check_type(node);
        expect(";");
        return node;
    }

    node = expr_stmt();
    expect(";");

    return node;
}

// declaration = basetype ident (( "[" num "]" ) | ( "=" expr ))?
Node *declaration() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LV;

    Type *ty = basetype();
    char *ident = expect_ident();
    ty = type_suffix(ty);

    Var *var = new_lvar(ty, ident);;
    var->offset = locals->offset + ty->size;

    node->var = var;

    if (consume("=")) {
        Node *n = calloc(1, sizeof(Node));
        n->kind = ND_EXPR_STMT;
        n->lhs = new_node(ND_AS, node, expr());
        return n;
    }

    return node;
}

// basetype = type "*"*
Type *basetype() {
    char *tw = expect_type();
    Type *type = int_type;
    if (strncmp(tw, "char", 4)==0)
        type = char_type;

    while (consume("*"))
        type = pointer_to(type);
    return type;
}

// type_suffix = ( "[" num "]" ( type_suffix )? )?
Type *type_suffix(Type *ty) {
    if (!consume("["))
        return ty;
    int num = expect_number();
    expect("]");
    ty = type_suffix(ty);
    return array_of(ty, num);
}

// expr_stmt = expr
Node *expr_stmt() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_EXPR_STMT;
    node->lhs = expr();
    return node;
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

// unary = ( '+' | '-' )? primary | ( '&' | '*' )? unary | "sizeof" unary | postfix
Node *unary() {
    if (consume("+"))
        return unary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), unary());
    if (consume("&"))
        return new_node_addr(unary());
    if (consume("*"))
        return new_node_deref(unary());

    if (consume("sizeof")) {
        Node *node = unary();
        check_type(node);
        return new_node_num(node->type->size);
    }

    return postfix();
}

// postfix = primary ( "[" expr "]" )*
Node *postfix() {
    Node *node = primary();

    while (consume("[")) {
        Node *idx = expr();
        expect("]");
        node = new_node_deref(new_add(node, idx));
    }

    return node;
}

// primary = '(' '{' stmt ( stmt )* '}' ')' // last stmt should be expr_stmt
//         | '(' expr ')' | ident ( "(" funcargs* )? | num | '"' str '"'
Node *primary() {
    if (consume("(")) {
        if (consume("{")) {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_STMT_EXPR;
            node->blocks = stmt();
            Node *cur = node->blocks;

            while (!consume("}")) {
                cur->next = stmt();
                cur = cur->next;
            }
            expect(")");

            if (cur->kind != ND_EXPR_STMT)
                error("statement expression returning void is not supported");
            return node;
        } else {
            Node *node = expr();
            expect(")");
            return node;
        }
    }

    Token *tok = consume_ident();
    if (tok) {
        if (consume("("))
            return new_node_func(strndup(tok->str, tok->len), funcargs());
        else
            if (find_var(tok))
                return new_node_var(find_var(tok));
    }

    tok = consume_str();
    if (tok)
        return new_node_var(new_str(tok));

    return new_node_num(expect_number());
}

// funcargs = ( add ( "," add )* )? ")"
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
