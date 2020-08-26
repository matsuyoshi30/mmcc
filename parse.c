#include "mmcc.h"

/// type

Type *void_type = &(Type){TY_VOID};
Type *char_type = &(Type){TY_CHAR, 1};
Type *short_type = &(Type){TY_SHORT, 2};
Type *int_type = &(Type){TY_INT, 4};
Type *long_type = &(Type){TY_LONG, 8};
Type *bool_type = &(Type){TY_BOOL, 1};
Type *enum_type = &(Type){TY_ENUM, 4};

bool is_integer(Type *type) {
    return type->kind == TY_INT || type->kind == TY_CHAR
        || type->kind == TY_SHORT || type->kind == TY_LONG
        || type->kind == TY_BOOL;
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
    case ND_MOD:
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

/// parser

Node *new_node(Nodekind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_break() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BREAK;
    return node;
}

Node *new_node_continue() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CONT;
    return node;
}

Node *new_node_goto() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_GOTO;
    return node;
}

Node *new_node_label() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LABEL;
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

Node *new_node_null_expr() {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NULL_EXPR;
    return node;
}

Node *new_node_not(Node *target) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NOT;
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

    // num - num
    if (is_integer(lhs->type) && is_integer(rhs->type))
        return new_node(ND_SUB, lhs, rhs);
    // ptr - ptr
    if (lhs->type->ptr_to && rhs->type->ptr_to)
        error_at(token->str, "invalid operands");
    // ptr - num
    rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size));

    return new_node(ND_SUB, lhs, rhs);
}

int scope_depth;

VarScope *varscope;

VarScope *push_varscope(char *name) {
    VarScope *v = calloc(1, sizeof(VarScope));
    v->name = name;
    v->depth = scope_depth;
    v->next = varscope;
    varscope = v;

    return v;
}

Var *locals;
Var *globals;

Var *new_params(Type *type, char *name) {
    Var *var = calloc(1, sizeof(Var));
    var->type = type;
    var->name = name;
    var->is_local = true;
    push_varscope(name)->var = var;

    return var;
}

Var *new_lvar(Type *type, char *name) {
    Var *var = calloc(1, sizeof(Var));
    var->type = type;
    var->name = name;
    var->is_local = true;
    var->next = locals;
    locals = var;
    push_varscope(name)->var = var;

    return var;
}

Var *new_gvar(Type *type, char *name, bool is_extern) {
    Var *var = calloc(1, sizeof(Var));
    var->type = type;
    var->name = name;
    var->is_local = false;
    var->is_static = true;
    push_varscope(name)->var = var;

    if (!is_extern) {
        var->next = globals;
        globals = var;
    }

    return var;
}

Var *find_var(Token *tok) {
    for (VarScope *vs=varscope; vs; vs=vs->next)
        if (strlen(vs->name) == tok->len && !strncmp(tok->str, vs->name, tok->len))
            return vs->var;

    return NULL;
}

Type *find_type(Token *tok) {
    if (tok->kind == TK_IDENT) {
        for (VarScope *vs=varscope; vs; vs=vs->next)
            if (strlen(vs->name) == tok->len && !strncmp(tok->str, vs->name, tok->len))
                return vs->def_type;
    }

    return NULL;
}

Var *strs;
static int lc = 0;

Var *new_str(Token *token) {
    Var *string = calloc(1, sizeof(Var));
    string->type = array_of(char_type, token->strlen);
    string->str = strndup(token->str, token->len);
    string->next = strs;
    string->lc = lc++;
    strs = string;

    return string;
}

TagScope *tagscope;

void push_tagscope(Tag *tag) {
    TagScope *t = calloc(1, sizeof(TagScope));
    t->tag = tag;
    t->depth = scope_depth;
    t->next = tagscope;
    tagscope = t;
}

Tag *tags;

TagScope *find_tag(Token *tok) {
    for (TagScope *ts=tagscope; ts; ts=ts->next) {
        if (strlen(ts->tag->name) == tok->len && !strncmp(tok->str, ts->tag->name, tok->len))
            return ts;
    }

    return NULL;
}

Node *current_switch;

void enter_scope() {
    scope_depth++;
}

void leave_scope() {
    scope_depth--;

    while (varscope && varscope->depth > scope_depth)
        varscope = varscope->next;

    while (tagscope && tagscope->depth > scope_depth)
        tagscope = tagscope->next;
}

// evaluate constant expression

long eval(Node *node) {
  switch (node->kind) {
  case ND_ADD:
    return eval(node->lhs) + eval(node->rhs);
  case ND_SUB:
    return eval(node->lhs) - eval(node->rhs);
  case ND_MUL:
    return eval(node->lhs) * eval(node->rhs);
  case ND_DIV:
    return eval(node->lhs) / eval(node->rhs);
  case ND_MOD:
    return eval(node->lhs) % eval(node->rhs);
  case ND_EQ:
    return eval(node->lhs) == eval(node->rhs);
  case ND_NE:
    return eval(node->lhs) != eval(node->rhs);
  case ND_MT:
    return eval(node->lhs) > eval(node->rhs);
  case ND_LT:
    return eval(node->lhs) < eval(node->rhs);
  case ND_OM:
    return eval(node->lhs) >= eval(node->rhs);
  case ND_OL:
    return eval(node->lhs) <= eval(node->rhs);
  case ND_NOT:
    return !eval(node->lhs);
  case ND_LOGOR:
    return eval(node->lhs) || eval(node->rhs);
  case ND_LOGAND:
    return eval(node->lhs) && eval(node->rhs);
  case ND_COND:
    return eval(node->cond) ? eval(node->then) : eval(node->els);
  case ND_COMMA:
    return eval(node->rhs);
  case ND_NUM:
    return node->val;
  }

  error_at(token->str, "invalid constant expression");
}

int const_expr();

Function *code;

void program();
Function *function(Type *type, char *funcname, bool is_extern);
Var *funcparams();
Node *stmt();
Node *typedefs();
Node *declaration();
bool is_typename();
Type *basetype();
Type *declarator(Type *basetype);
Type *type_suffix(Type *ty);
Member *struct_members();
Type *struct_decl();
Type *enum_decl();
Node *expr_stmt();
Node *expr();
Node *assign();
Node *conditional();
Node *logicOr();
Node *logicAnd();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *cast();
Node *unary();
Node *postfix();
Node *struct_ref();
Member *get_struct_member();
Node *primary();
Node *funcargs();
Node *read_array();

// global variable initializer

Initializer *new_init_gval(Initializer *cur, int size, long val) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->size = size;
    init->val = val;
    cur->next = init;

    return init;
}

Initializer *new_init_glabel(Initializer *cur, char *label) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->label = label;
    cur->next = init;

    return init;
}

Initializer *gvar_initializer_helper(Initializer *cur, Type *type) {
    Node *expr = conditional();

    // another variable's address
    if (expr->kind == ND_ADDR) {
        if (expr->lhs->kind != ND_LV)
            error_at(token->str, "invalid initializer about variable's address");
        return new_init_glabel(cur, expr->lhs->var->name);
    }

    // constant expression
    return new_init_gval(cur, type->size, eval(expr));
}

Initializer *gvar_initializer(Type *type) {
    Initializer head;
    gvar_initializer_helper(&head, type);

    return head.next;
}

// program = ( basetype ident ( function | gvar ";" ) )* | typedefs
void program() {
    Function head;
    head.next = NULL;
    Function *cur = &head;

    strs = NULL;
    globals = calloc(1, sizeof(Var));
    scope_depth = 0;

    while (!at_eof()) {
        if (peek("typedef")) {
            typedefs();
        } else {
            bool is_static = consume("static");
            bool is_extern = consume("extern");

            Type *base = basetype();
            if (consume(";"))
                continue;
            Type *type = declarator(base);
            char *name = type->name;

            locals = NULL;
            tags = NULL;
            if (peek("(")) {
                Function *func = function(type, name, is_extern);
                if (func) {
                    cur->next = func;
                    cur->next->is_static = is_static;
                    cur = cur->next;
                } else {
                    continue;
                }
            } else {
                Var *var = new_gvar(type, name, is_extern);
                if (consume("="))
                    var->initializer = gvar_initializer(type);
                expect(";");
            }
        }
    }

    code = head.next;
}

// function = "(" funcparams* ")" "{" stmt* "}"
Function *function(Type *type, char *funcname, bool is_extern) {
    Function *func = calloc(1, sizeof(Function));

    expect("(");

    locals = funcparams();
    if (consume(";")) {
        return NULL;
    }

    func->params = locals;
    func->type = type;
    func->name = funcname;

    if (is_extern)
        error_at(token->str, "invalid definition");

    expect("{");

    Node head;
    head.next = NULL;
    Node *cur = &head;
    while (!consume("}")) {
        cur->next = stmt();
        check_type(cur->next);
        cur = cur->next;
    }

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

// stmt = "return" expr? ";"
//        | "{" stmt* "}"
//        | "if" "(" expr ")" stmt ( "else" stmt )?
//        | "while" "(" expr ")" stmt
//        | "for" "(" expr_stmt? ";" expr? ";" expr_stmt? ")" stmt
//        | "switch" "(" expr ")" stmt
//        | "case" const_expr ":" stmt
//        | "default" ":" stmt
//        | "break" ";"
//        | "continue" ";"
//        | "goto" ident ";"
//        | ident ":" stmt
//        | declaration
//        | expr_stmt ";"
Node *stmt() {
    Node *node;

    if (consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RET;
        if (consume(";"))
            return node;
        node->lhs = expr();
        expect(";");
        return node;
    }

    if (consume("{")) {
        Node head;
        head.next = NULL;
        Node *cur = &head;

        enter_scope();

        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        while (!consume("}")) {
            cur->next = stmt();
            check_type(cur->next);
            cur = cur->next;
        }
        node->blocks = head.next;

        leave_scope();

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
        node->kind = ND_FOR;
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
            if (is_typename()) {
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

    if (consume("switch")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_SWITCH;
        expect("(");
        node->cond = expr();
        check_type(node->cond);
        expect(")");

        Node *cw = current_switch;
        current_switch = node;
        node->then = stmt();
        current_switch = cw;
        return node;
    }

    if (consume("case")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_CASE;
        node->val = const_expr();
        expect(":");
        node->lhs = stmt();
        node->next_case = current_switch->next_case;
        current_switch->next_case = node;
        return node;
    }

    if (consume("default")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_CASE;
        expect(":");
        node->lhs = stmt();
        current_switch->default_case = node;
        return node;
    }

    if (consume("break")) {
        expect(";");
        return new_node_break();
    }

    if (consume("continue")) {
        expect(";");
        return new_node_continue();
    }

    if (consume("goto")) {
        Token *tok = consume_ident();
        node = new_node_goto();
        node->labelname = strndup(tok->str, tok->len);
        expect(";");
        return node;
    }

    if (token->kind == TK_IDENT) {
        Token *tok = consume_ident();
        if (consume(":")) {
            node = new_node_label();
            node->labelname = strndup(tok->str, tok->len);
            node->lhs = stmt();
            return node;
        }
        token = tok;
    }

    if (peek("typedef")) {
        node = typedefs();
        return node;
    }

    if (is_typename()) {
        node = declaration();
        check_type(node);
        return node;
    }

    node = expr_stmt();
    expect(";");

    return node;
}

// typedefs = "typedef" basetype declarator ( "," declarator ) ";"
Node *typedefs() {
    expect("typedef");

    Type *base = basetype();

    int num = 0;
    while (!consume(";")) {
        if (num > 0)
            expect(",");

        Type *type = declarator(base);
        push_varscope(type->name)->def_type = type;

        num++;
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;

    return node;
}

// local variable initializer

typedef struct Designator Designator;
struct Designator {
    Designator *next;
    int idx;
};

Node *new_designator_helper(Var *var, Designator *desg) {
    if (!desg)
        return new_node_var(var);

    Node *node = new_designator_helper(var, desg->next);
    node = new_add(node, new_node_num(desg->idx));
    return new_node_deref(node);
}

Node *new_designator(Var *var, Designator *desg, Node *rhs) {
    Node *lhs = new_designator_helper(var, desg);
    Node *node = new_node(ND_AS, lhs, rhs);

    Node *ret = calloc(1, sizeof(Node));
    ret->kind = ND_EXPR_STMT;
    ret->lhs = node;
    return ret;
}

Node *lvar_init_zero(Node *cur, Var *var, Type *type, Designator *desg) {
  if (type->kind == TY_ARR) {
    for (int i = 0; i < type->size_array; i++) {
      Designator desg2 = {desg, i++};
      cur = lvar_init_zero(cur, var, type->ptr_to, &desg2);
    }
    return cur;
  }

  cur->next = new_designator(var, desg, new_node_num(0));
  return cur->next;
}

Node *lvar_initializer_helper(Node *cur, Var *var, Type *type, Designator *desg) {
    // char array can be initialized by a string literal
    // e.g.) char x[4] = "foo" -> char x[4] = {'f', 'o', 'o', '\0')
    if (type->kind == TY_ARR && type->ptr_to->kind == TY_CHAR
        && token->kind == TK_STR) {
        Token *tok = consume_str();

        if (type->is_incomplete) {
            type->size = tok->strlen;
            type->size_array = tok->strlen;
            type->is_incomplete = false;
        }

        int len = tok->strlen;
        if (len > type->size_array)
            len = type->size_array;

        for (int i=0; i<len; i++) {
            Designator desg2 = {desg, i};
            Node *rhs = new_node_num(tok->str[i]);
            cur->next = new_designator(var, &desg2, rhs);
            cur = cur->next;
        }

        for (int i=len; i < type->size_array; i++) {
            Designator desg2 = {desg, i};
            cur = lvar_init_zero(cur, var, type->ptr_to, &desg2);
        }

        return cur;
    }

    if (type->kind == TY_ARR) {
        expect("{");
        int i = 0;

        do {
            Designator desg2 = {desg, i++};
            cur = lvar_initializer_helper(cur, var, type->ptr_to, &desg2);
        } while (!peek_end() && consume(","));

        if (consume(","))
            expect("}");
        else
            expect("}");

        if (type->is_incomplete) {
            type->size = type->ptr_to->size * i;
            type->size_array = i;
            type->is_incomplete = false;
        }

        return cur;
    }

    cur->next = new_designator(var, desg, assign());
    return cur->next;
}

Node *lvar_initializer(Var *var) {
    Node head;
    lvar_initializer_helper(&head, var, var->type, NULL);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->blocks = head.next;
    return node;
}

// declaration = basetype declarator ( "=" ( lvar_initializer ) )? ( "," declarator ( "=" ( lvar_initializer ) )? )* ";"
Node *declaration() {
    Node head;
    head.next = NULL;
    Node *cur = &head;

    Type *base = basetype();

    int num = 0;
    while (!consume(";")) {
        if (num > 0)
            expect(",");

        Type *type = declarator(base);

        Var *var = new_lvar(type, type->name);
        var->offset = locals->offset + type->size;

        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LV;
        node->var = var;

        if (consume("=")) {
            cur->next = lvar_initializer(var);
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

// is_typename = "int" | "char" | "short" | "long" | "_Bool"
//             | "struct" | "void" | typedef_name | "enum"
bool is_typename() {
    if (peek("int") || peek("char") || peek("short") || peek("long") || peek("_Bool")
        || peek("struct") || peek("void") || find_type(token) || peek("enum"))
        return true;

    return false;
}

// basetype = is_typename | typedef_name
Type *basetype() {
    if (!is_typename())
        error_at(token->str, "expected typename");

    if (consume("void"))
        return void_type;
    if (consume("int"))
        return int_type;
    if (consume("char"))
        return char_type;
    if (consume("short"))
        return short_type;
    if (consume("long"))
        return long_type;
    if (consume("_Bool"))
        return bool_type;
    if (consume("struct"))
        return struct_decl();
    if (consume("enum"))
        return enum_decl();

    Token *tok = consume_ident();
    if (tok)
        return find_type(tok);

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
    Token *tok = consume_ident();
    char *name = strndup(tok->str, tok->len);
    type = type_suffix(type);
    type->name = name;

    return type;
}

// abstract_declarator = "*"* "(" abstract_declarator ")" ( type_suffix )?
Type *abstract_declarator(Type *basetype) {
    Type *type = basetype;
    while (consume("*"))
        type = pointer_to(type);

    if (consume("(")) {
        Type *placeholder = calloc(1, sizeof(Type));
        Type *nestedType = abstract_declarator(placeholder);
        expect(")");
        *placeholder = *type_suffix(type);
        return nestedType;
    }

    return type_suffix(type);
}

// type_suffix = ( "[" const_expr? "]" ( type_suffix )? )?
Type *type_suffix(Type *ty) {
    if (!consume("["))
        return ty;

    int num;
    bool is_incomplete = true;
    if (!consume("]")) {
        num = const_expr();
        is_incomplete = false;
        expect("]");
    }

    ty = type_suffix(ty);
    if (ty->is_incomplete)
        error_at(token->str, "incomplete token type");
    ty = array_of(ty, num);
    ty->is_incomplete = is_incomplete;

    return ty;
}

// struct_decl = ident | ident "{" struct_member* | "{" struct_member*
Type *struct_decl() {
    Token *tok = consume_ident();
    if (tok) {
        char *name = strndup(tok->str, tok->len);
        TagScope *tsc;

        if (consume("{")) {
            Type *type = calloc(1, sizeof(Type));
            type->kind = TY_STRUCT;
            type->members = struct_members();

            int offset = 0;
            for (Member *m=type->members; m; m=m->next) {
                m->offset = offset;
                offset += m->type->size;
            }
            type->size = offset;

            tsc = find_tag(tok);
            // redefinition
            if (tsc && tsc->depth == scope_depth) {
                *tsc->tag->type = *type;
                return type;
            }

            Tag *tag = calloc(1, sizeof(Tag));
            tag->name = name;
            tag->type = type;
            tag->next = tags;
            tags = tag;
            push_tagscope(tags);

            return type;
        } else {
            tsc = find_tag(tok);
            if (!tsc) {
                Tag *tag = calloc(1, sizeof(Tag));

                Type *type = calloc(1, sizeof(Type));
                type->kind = TY_STRUCT;
                tag->name = name;
                tag->type = type;
                tag->next = tags;
                tags = tag;
                push_tagscope(tags);

                return type;
            }

            return tsc->tag->type;
        }
    }

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

// enum_decl = ident? "{" enum-list* "}"
// enum-list = ident ( "=" const_expr )? ( "," ident ( "=" const_expr )? )*
Type *enum_decl() {
    Token *tok = consume_ident();

    TagScope *tsc;
    if (tok) {
        if (!peek("{")) {
            tsc = find_tag(tok);
            if (!tsc)
                error_at(token->str, "unknown tag name");
            if (tsc->tag->type->kind != TY_ENUM)
                error_at(token->str, "not enum type");
            return tsc->tag->type;
        }
    }

    expect("{");

    // read enum-list
    int num = 0;
    int val = 0;
    while (!consume("}")) {
        if (num > 0)
            expect(",");

        Token *tok = consume_ident();
        char *name = strndup(tok->str, tok->len);
        Var *var = new_lvar(enum_type, name);
        var->offset = locals->offset + var->type->size;

        if (consume("=")) {
            val = const_expr();
        }

        var->enum_val = val++;

        tok = token;
        if (consume(",") && peek("}")) {
            consume("}");
            break;
        }
        token = tok;
        num++;
    }

    if (tok) {
        Tag *tag = calloc(1, sizeof(Tag));
        tag->name = strndup(tok->str, tok->len);
        tag->type = enum_type;
        tag->next = tags;
        tags = tag;
        push_tagscope(tags);
    }

    return enum_type;
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

// x += y => tmp = &x, *tmp = *tmp + y
Node *compound_assign(Node *node, Nodekind kind) {
    check_type(node);
    Var *tmp = new_lvar(pointer_to(node->type), "");

    Node *e1 = new_node(ND_AS, new_node_var(tmp), new_node_addr(node));
    Node *e2 = new_node(ND_AS,
                        new_node_deref(new_node_var(tmp)),
                        new_node(kind, new_node_deref(new_node_var(tmp)), assign()));

    return new_node(ND_COMMA, e1, e2);
}

// assign = conditional ( "=" assign | "+=" assign | "-=" assign | "*=" assign | "/=" assign | "%=" assign )?
Node *assign() {
    Node *node = conditional();

    if (consume("="))
        node = new_node(ND_AS, node, assign());
    if (consume("+="))
        node = compound_assign(node, ND_ADD);
    if (consume("-="))
        node = compound_assign(node, ND_SUB);
    if (consume("*="))
        node = compound_assign(node, ND_MUL);
    if (consume("/="))
        node = compound_assign(node, ND_DIV);
    if (consume("%="))
        node = compound_assign(node, ND_MOD);

    return node;
}

int const_expr() {
  return eval(conditional());
}

// conditional = logicOr ( "?" expr ":" conditional )?
Node *conditional() {
    Node *node = logicOr();

    if (consume("?")) {
        Node *node2 = calloc(1, sizeof(Node));
        node2->kind = ND_COND;
        node2->cond = node;
        check_type(node2->cond);
        node2->then = expr();
        expect(":");
        node2->els = conditional();
        check_type(node2->els);
        return node2;
    }

    return node;
}

// logicOr = logicAnd ( "||" logicAnd )?
Node *logicOr() {
    Node *node = logicAnd();

    if (consume("||"))
        node = new_node(ND_LOGOR, node, logicAnd());

    return node;
}

// logicAnd = equality ( "&&" equality )?
Node *logicAnd() {
    Node *node = equality();

    if (consume("&&"))
        node = new_node(ND_LOGAND, node, equality());

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

// mul = cast ( '*' cast | '/' cast | '%' cast )*
Node *mul() {
    Node *node = cast();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, cast());
        else if (consume("/"))
            node = new_node(ND_DIV, node, cast());
        else if (consume("%"))
            node = new_node(ND_MOD, node, cast());
        else
            return node;
    }
}

// cast = "(" typename ")" cast | unary
Node *cast() {
    Token *tok = token;
    if (consume("(")) {
        if (is_typename()) {
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_CAST;
            Type *type = basetype();
            node->type = abstract_declarator(type);
            expect(")");
            node->lhs = cast();
            check_type(node->lhs);
            return node;
        }
        token = tok; // bring back the token sequense
    }

    return unary();
}

// unary = ( '+' | '-' )? cast
//         | ( '&' | '*' | '!' )? cast
//         | "sizeof" unary
//         | ( "++" | "--" ) unary
//         | postfix
Node *unary() {
    if (consume("+"))
        return cast();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), cast());
    if (consume("&"))
        return new_node_addr(cast());
    if (consume("*"))
        return new_node_deref(cast());
    if (consume("!"))
        return new_node_not(cast());

    if (consume("++")) {
        Node *node = unary();
        return new_node(ND_COMMA, new_node(ND_AS, node, new_add(node, new_node_num(1))), node);
    }
    if (consume("--")) {
        Node *node = unary();
        return new_node(ND_COMMA, new_node(ND_AS, node, new_add(node, new_node_num(-1))), node);
    }

    if (consume("sizeof")) {
        Token *tok = token;
        if (consume("(")) {
            if (is_typename()) {
                Type *type = basetype();
                type = abstract_declarator(type);
                expect(")");
                return new_node_num(type->size);
            }
            token = tok; // bring back the token sequense
        }

        Node *node = unary();
        check_type(node);
        return new_node_num(node->type->size);
    }

    return postfix();
}

// x++ => tmp = &x, *tmp = *tmp + 1, *tmp - 1
// x-- => tmp = &x, *tmp = *tmp - 1, *tmp + 1
Node *postop(Node *node, int val) {
    check_type(node);
    Var *tmp = new_lvar(pointer_to(node->type), "");

    Node *e1 = new_node(ND_AS, new_node_var(tmp), new_node_addr(node));
    Node *e2 = new_node(ND_AS,
                        new_node_deref(new_node_var(tmp)),
                        new_node(ND_ADD, new_node_deref(new_node_var(tmp)), new_node_num(val)));
    Node *e3 = new_add(new_node_deref(new_node_var(tmp)), new_node_num(val*-1));

    return new_node(ND_COMMA, e1, new_node(ND_COMMA, e2, e3));
}

// postfix = primary ( ( "[" expr "]" ) | ( "." ident ) | ( "->" ident ) | "++" | "--" )*
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
            Token *tok = consume_ident();
            char *ident = strndup(tok->str, tok->len);
            node = struct_ref(node, ident);
            continue;
        }

        if (consume("->")) {
            // b->n => (*b).n
            Token *tok = consume_ident();
            char *ident = strndup(tok->str, tok->len);
            node = struct_ref(new_node_deref(node), ident);
            continue;
        }

        if (consume("++")) {
            node = postop(node, 1);
            continue;
        }

        if (consume("--")) {
            node = postop(node, -1);
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
//         | ident ( "(" funcargs* )?
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
            Var *var = find_var(tok);
            if (var->type->kind == TY_ENUM) {
                return new_node_num(var->enum_val);
            }
            return new_node_var(var);
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
