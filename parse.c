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
        if (!node->lhs->type->ptr_to) {
            nop();
            error("invalid pointer dereference");
        }
        node->type = node->lhs->type->ptr_to;
        return;
    case ND_STMT_EXPR: {
        Node *block = node->blocks;
        while (block->next)
            block = block->next;
        node->type = block->type;
        return;
    }
    case ND_COMMA:
        node->type = node->rhs->type;
        return;
    case ND_MEMBER:
        node->type = node->member->type;
        return;
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
        error_at(token->str, "invalid operands");
    // num + ptr -> ptr + num
    if (rhs->type->ptr_to) {
        Node *temp = lhs;
        lhs = rhs;
        rhs = temp;
    }
    // ptr + num
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
        error_at(token->str, "invalid operands");
    // ptr + num
    rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size));

    return new_node(ND_SUB, lhs, rhs);
}

Var *locals;
Var *globals;

Var *new_params(Type *type, char *name) {
    Var *var = calloc(1, sizeof(Var));
    var->type = type;
    var->name = name;
    var->is_local = true;

    return var;
}

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
Type *declarator(Type *basetype);
Type *type_suffix(Type *ty);
Member *struct_members();
Type *struct_decl();
Node *expr_stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *postfix();
Node *struct_ref();
Member *get_struct_member();
Node *primary();
Node *funcargs();
Node *read_array();

// program = ( basetype ident ( function | gvar ";" ) )*
void program() {
    Function head;
    head.next = NULL;
    Function *cur = &head;

    strs = NULL;
    globals = calloc(1, sizeof(Var));

    while (!at_eof()) {
        Type *ty = basetype();
        while (consume("*"))
            ty = pointer_to(ty);
        char *name = expect_ident();

        locals = NULL;
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

// funcparams = ( basetype declarator ( "," basetype declarator )* )? ")"
Var *funcparams() {
    Var head;
    head.next = NULL;
    Var *cur = &head;

    int num = 0;
    while (!consume(")")) {
        if (num > 0)
            expect(",");

        Type *base = basetype();
        Type *type = declarator(base);
        char *name = type->name;

        Var *var = new_params(type, name);
        var->offset = cur->offset + type->size;

        cur->next = var;

        num++;
        cur = cur->next;
    }

    return head.next;
}

// stmt = "return" expr ";"
//        | "{" stmt* "}"
//        | "if" "(" expr ")" stmt ( "else" stmt )?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr_stmt? ";" expr? ";" expr_stmt? ")" stmt
//        | declaration
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
            if (peek("int") || peek("char")) {
                node->preop = declaration();
            } else {
                node->preop = expr_stmt();
                expect(";");
            }
            check_type(node->preop);
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

    if (peek("int") || peek("char") || peek("struct")) {
        node = declaration();
        check_type(node);
        return node;
    }

    node = expr_stmt();
    expect(";");

    return node;
}

// declaration = basetype declarator ( "=" expr )? ( "," declarator ( "=" expr )? )? ";"
Node *declaration() {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    Type *ty = basetype();

    int num = 0;
    while (!consume(";")) {
        if (num > 0)
            expect(",");

        Type *type = declarator(ty);

        Var *var = new_lvar(type, type->name);
        var->offset = locals->offset + type->size;

        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LV;
        node->var = var;

        if (consume("=")) {
            Node *n = calloc(1, sizeof(Node));
            n->kind = ND_EXPR_STMT;
            n->lhs = new_node(ND_AS, node, assign());
            cur->next = n;
        } else {
            cur->next = node;
        }

        num++;
        cur = cur->next;
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->blocks = head.next;

    return node;
}

// basetype = "int" | "char" | struct_decl
Type *basetype() {
    char *tw = expect_type();
    if (strcmp(tw, "int") == 0)
        return int_type;
    if (strcmp(tw, "char") == 0)
        return char_type;
    if (strcmp(tw, "struct") == 0)
        return struct_decl();

    return NULL;
}

// declarator = "*"* ( "(" declarator ")" | ident ) ( type_suffix )?
Type *declarator(Type *basetype) {
    Type *type = basetype;
    while (consume("*"))
        type = pointer_to(type);

    // e.g.) int (*x)[3];
    // -> (*x)       >>> * ___
    // -> int _[num] >>> [] int // placeholder
    // => * [] int
    if (consume("(")) {
        Type *placeholder = calloc(1, sizeof(Type));
        Type *nestedType = declarator(placeholder);
        expect(")");
        *placeholder = *type_suffix(type);
        return nestedType;
    }
    char *name = expect_ident();
    type = type_suffix(type);
    type->name = name;

    return type;
}

// type_suffix = ( "[" num "]" ( type_suffix )? )?
Type *type_suffix(Type *ty) {
    if (consume("[")) {
        int num = expect_number();
        expect("]");
        ty = type_suffix(ty);
        return array_of(ty, num);
    }

    return ty;
}

// struct_decl = "{" struct_member*
Type *struct_decl() {
    expect("{");

    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_STRUCT;
    type->members = struct_members();

    // calculate offset and struct size
    int offset = 0;
    for (Member *m=type->members; m; m=m->next) {
        m->offset = offset;
        offset += m->type->size;
    }
    type->size = offset;

    return type;
}

// struct_member = ( basetype declarator ( "," declarator )* ";" )* "}"
Member *struct_members() {
    Member head;
    head.next = NULL;
    Member *cur = &head;

    while (!consume("}")) {
        int num = 0;
        Type *base = basetype();
        while (!consume(";")) {
            if (num > 0)
                expect(",");

            Member *member = calloc(1, sizeof(Member));
            member->type = declarator(base);
            member->name = member->type->name;
            cur->next = member;

            num++;
            cur = cur->next;
        }
    }

    return head.next;
}

// expr_stmt = expr
Node *expr_stmt() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_EXPR_STMT;
    node->lhs = expr();
    return node;
}

// expr = assign ( "," assign )*
Node *expr() {
    Node *node = assign();

    while (consume(","))
        node = new_node(ND_COMMA, node, assign());

    return node;
}

// assign = equality ( "=" assign | "+=" assign | "-=" assign | "*=" assign | "/=" assign )?
Node *assign() {
    Node *node = equality();

    if (consume("="))
        node = new_node(ND_AS, node, assign());
    if (consume("+="))
        node = new_node(ND_AS, node, new_node(ND_ADD, node, assign()));
    if (consume("-="))
        node = new_node(ND_AS, node, new_node(ND_SUB, node, assign()));
    if (consume("*="))
        node = new_node(ND_AS, node, new_node(ND_MUL, node, assign()));
    if (consume("/="))
        node = new_node(ND_AS, node, new_node(ND_DIV, node, assign()));

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

// postfix = primary ( ( "[" expr "]" ) | ( "." ident ) )*
Node *postfix() {
    Node *node = primary();

    while (1) {
        if (consume("[")) {
            Node *idx = expr();
            expect("]");
            node = new_node_deref(new_add(node, idx));
            continue;
        }

        if (consume(".")) {
            // access struct member
            char *ident = expect_ident();
            node = struct_ref(node, ident);
            continue;
        }

        return node;
    }
}

Node *struct_ref(Node *node, char *ident) {
    check_type(node);
    if (node->type->kind != TY_STRUCT)
        error("not struct");

    Node *n = calloc(1, sizeof(Node));
    n->kind = ND_MEMBER;
    n->lhs = node;
    n->member = get_struct_member(node->type, ident);

    return n;
}

Member *get_struct_member(Type *type, char *name) {
    for (Member *m=type->members; m; m=m->next) {
        if (strcmp(m->name, name) == 0)
            return m;
    }

    error("no such member");
}

// primary = '(' '{' stmt ( stmt )* '}' ')' // last stmt should be expr_stmt
//         | '(' expr ')'
//         | ident (( "(" funcargs* )? | ('++' | '--'))
//         | num | '"' str '"'
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
                error_at(token->str, "statement expression returning void is not supported");
            return node;
        } else {
            Node *node = expr();
            expect(")");
            return node;
        }
    }

    Token *tok = consume_ident();
    if (tok) {
        if (consume("(")) {
            return new_node_func(strndup(tok->str, tok->len), funcargs());
        } else {
            Node *node =  new_node_var(find_var(tok));
            Node *rhs = calloc(1, sizeof(Node));
            if (consume("++")) {
                rhs->kind = ND_ADD;
                rhs->lhs = node;
                rhs->rhs = new_node_num(1);
                node = new_node(ND_AS, node, rhs);
            } else if (consume("--")) {
                rhs->kind = ND_SUB;
                rhs->lhs = node;
                rhs->rhs = new_node_num(1);
                node = new_node(ND_AS, node, rhs);
            }

            return node;
        }
    }

    tok = consume_str();
    if (tok)
        return new_node_var(new_str(tok));

    return new_node_num(expect_number());
}

// funcargs = ( assign ( "," assign )* )? ")"
Node *funcargs() {
    if (consume(")"))
        return NULL; // no argument

    Node *head = assign();
    Node *cur = head;
    while (consume(",")) {
        cur->next = assign();
        cur = cur->next;
    }
    expect(")");

    return head;
}
