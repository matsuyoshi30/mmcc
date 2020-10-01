#include "mmcc.h"

Node *new_node(Nodekind kind, Node *lhs, Node *rhs, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    node->tok = tok;
    check_type(node);
    return node;
}

Node *new_node_break(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BREAK;
    node->tok = tok;
    return node;
}

Node *new_node_continue(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CONT;
    node->tok = tok;
    return node;
}

Node *new_node_goto(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_GOTO;
    node->tok = tok;
    return node;
}

Node *new_node_label(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LABEL;
    node->tok = tok;
    return node;
}

Node *new_node_addr(Node *target, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_ADDR;
    node->lhs = target;
    node->tok = tok;
    return node;
}

Node *new_node_deref(Node *target, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEREF;
    node->lhs = target;
    node->tok = tok;
    return node;
}

Node *new_node_null_expr(Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NULL_EXPR;
    node->tok = tok;
    return node;
}

Node *new_node_not(Node *target, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NOT;
    node->lhs = target;
    node->tok = tok;
    return node;
}

Node *new_node_func(char *funcname, Node *args, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC;
    node->tok = tok;
    node->funcname = funcname;
    node->args = args;
    return node;
}

Node *new_node_var(Var *var, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LV;
    node->tok = tok;
    node->var = var;
    check_type(node);
    return node;
}

Node *new_node_num(int val, Token *tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->tok = tok;
    node->val = val;
    return node;
}

Node *new_add(Node *lhs, Node *rhs, Token *tok) {
    check_type(lhs);
    check_type(rhs);

    // num + num
    if (is_integer(lhs->type) && is_integer(rhs->type))
        return new_node(ND_ADD, lhs, rhs, tok);
    // ptr + ptr
    if (lhs->type->ptr_to && rhs->type->ptr_to)
        error_tok(tok, "invalid operands");
    // num + ptr -> ptr + num
    if (rhs->type->ptr_to) {
        Node *temp = lhs;
        lhs = rhs;
        rhs = temp;
    }
    // ptr + num
    rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size, tok), tok);

    return new_node(ND_ADD, lhs, rhs, tok);
}

Node *new_sub(Node *lhs, Node *rhs, Token *tok) {
    check_type(lhs);
    check_type(rhs);

    // num - num
    if (is_integer(lhs->type) && is_integer(rhs->type))
        return new_node(ND_SUB, lhs, rhs, tok);
    // ptr - ptr, which returns how many elements are between the two
    if (lhs->type->ptr_to && rhs->type->ptr_to) {
        Node *node = new_node(ND_SUB, lhs, rhs, tok);
        return new_node(ND_DIV, node, new_node_num(lhs->type->ptr_to->size, tok), tok);
    }
    // ptr - num
    rhs = new_node(ND_MUL, rhs, new_node_num(lhs->type->ptr_to->size, tok), tok);

    return new_node(ND_SUB, lhs, rhs, tok);
}

static int scope_depth;

static VarScope *varscope;

VarScope *push_varscope(char *name) {
    VarScope *v = calloc(1, sizeof(VarScope));
    v->name = name;
    v->depth = scope_depth;
    v->next = varscope;
    varscope = v;

    return v;
}

static Var *locals;
static Var *globals;

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

Var *new_gvar(Type *type, char *name, bool is_extern, bool is_static) {
    Var *var = calloc(1, sizeof(Var));
    var->type = type;
    var->name = name;
    var->is_local = false;
    var->is_static = is_static;
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

char *new_label() {
    static int cnt = 0;
    char name[16];
    snprintf(name, 16, ".LC%d", cnt++);
    return strndup(name, 16);
}

static TagScope *tagscope;

void push_tagscope(Tag *tag) {
    TagScope *t = calloc(1, sizeof(TagScope));
    t->tag = tag;
    t->depth = scope_depth;
    t->next = tagscope;
    tagscope = t;
}

static Tag *tags;

TagScope *find_tag(Token *tok) {
    for (TagScope *ts=tagscope; ts; ts=ts->next) {
        if (strlen(ts->tag->name) == tok->len && !strncmp(tok->str, ts->tag->name, tok->len))
            return ts;
    }

    return NULL;
}

static Node *current_switch;

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
  case ND_LV:
    return node->var->enum_val;
  case ND_NUM:
    return node->val;
  }

  error_tok(node->tok, "invalid constant expression");
}

int const_expr();

static Function *code;

Program *program();
Function *function(Type *type, char *funcname, bool is_extern);
void funcparams(Function *func);
Node *stmt();
void *typedefs();
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
Node *compound_literal();
Node *struct_ref();
Member *get_struct_member();
Node *primary();
Node *funcargs();

// global variable initializer

Initializer *new_init_gval(Initializer *cur, int size, long val) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->size = size;
    init->val = val;
    cur->next = init;

    return init;
}

Initializer *new_init_zero(Initializer *cur, int size) {
    for (int i=0; i<size; i++)
        cur = new_init_gval(cur, 1, 0);

    return cur;
}

Initializer *emit_struct_padding(Initializer *cur, Type *type, Member *member) {
    int start = member->offset + member->type->size;
    int end = member->next ? member->next->offset : type->size;
    return new_init_zero(cur, end - start);
}

Initializer *new_init_glabel(Initializer *cur, char *labelname) {
    Initializer *init = calloc(1, sizeof(Initializer));
    init->label = labelname;
    cur->next = init;

    return init;
}

Initializer *new_init_string(Token *tok) {
    Initializer head = {};
    Initializer *cur = &head;

    char *s = strndup(tok->str, tok->len);
    for (int i=0; i<strlen(s); i++)
        cur = new_init_gval(cur, 1, s[i]);
    cur = new_init_gval(cur, 1, 0); // null-terminated
    return head.next;
}

void skip_excess_elements_helper() {
    for (;;) {
        if (consume("{"))
            skip_excess_elements_helper();
        else
            assign();

        if (consume("}") || (consume(",") && consume("}")))
            return;
        expect(",");
    }
}

void skip_excess_elements() {
    expect(",");
    skip_excess_elements_helper();
}

bool is_excess_array_size(Type *type, int cnt) {
    if (type->is_incomplete)
        return false;
    return cnt >= type->size;
}

Initializer *gvar_initializer_helper(Initializer *cur, Type *type) {
    if (type->kind == TY_ARR && type->ptr_to->kind == TY_CHAR && token->kind == TK_STR) {
        Token *tok = consume_str();

        if (type->is_incomplete) {
            type->size = tok->strlen;
            type->size_array = tok->strlen;
            type->is_incomplete = false;
        }

        int len = tok->strlen;
        if (len > type->size_array)
            len = type->size_array;

        for (int i=0; i<len; i++)
            cur = new_init_gval(cur, 1, tok->str[i]);

        return cur;
    }

    // array
    if (type->kind == TY_ARR) {
        bool open = consume("{");
        int elem_cnt = 0;

        if (!peek("}")) {
            do {
                cur = gvar_initializer_helper(cur, type->ptr_to);
                elem_cnt++;
            } while (!is_excess_array_size(type, elem_cnt) && !peek_end() && consume(","));
        }

        if (open && !(consume("}") || (consume(",") && consume("}"))))
            skip_excess_elements();

        if (type->is_incomplete) {
            type->size = type->ptr_to->size * elem_cnt;
            type->size_array = elem_cnt;
            type->is_incomplete = false;
        }

        // padding extra array elements with zero value
        cur = new_init_zero(cur, type->ptr_to->size * (type->size_array - elem_cnt));

        return cur;
    }

    // struct
    if (type->kind == TY_STRUCT) {
        bool open = consume("{");
        Member *mem = type->members;

        if (!peek("}")) {
            do {
                cur = gvar_initializer_helper(cur, mem->type);
                cur = emit_struct_padding(cur, type, mem);
                mem = mem->next;
            } while (mem && !peek_end() && consume(","));
        }

        if (open && !(consume("}") || (consume(",") && consume("}"))))
            skip_excess_elements();

        // set excess struct members to zero
        if (mem)
            cur = new_init_zero(cur, type->size - mem->offset);

        return cur;
    }

    bool open_brace = consume("{");
    Node *expr = conditional();
    if (open_brace)
        expect("}");

    // another variable's address
    if (expr->kind == ND_ADDR) {
        if (expr->lhs->kind != ND_LV)
            error_tok(token, "invalid initializer about variable's address");
        return new_init_glabel(cur, expr->lhs->var->name);
    }

    // string literal
    if (expr->kind == ND_LV && expr->var->type->kind == TY_ARR)
        return new_init_glabel(cur, expr->var->name);

    // constant expression
    return new_init_gval(cur, type->size, eval(expr));
}

Initializer *gvar_initializer(Type *type) {
    Initializer head = {};
    gvar_initializer_helper(&head, type);

    return head.next;
}

// program = ( basetype ident ( function | gvar ";" ) )* | typedefs
Program *program() {
    Function head = {};
    Function *cur = &head;

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
                Var *var = new_gvar(type, name, is_extern, is_static);
                if (consume("="))
                    var->initializer = gvar_initializer(type);
                expect(";");
            }
        }
    }

    Program *prog = calloc(1, sizeof(Program));
    prog->globals = globals;
    prog->code = head.next;
    return prog;
}

// function = "(" funcparams* ")" "{" stmt* "}"
Function *function(Type *type, char *funcname, bool is_extern) {
    // add a function type to the scope
    new_gvar(func_type(type), funcname, true, true);

    Function *func = calloc(1, sizeof(Function));

    enter_scope();
    expect("(");

    funcparams(func);
    if (consume(";")) {
        leave_scope();
        return NULL;
    }

    func->params = locals;
    func->type = type;
    func->name = funcname;

    if (is_extern)
        error_tok(token, "invalid definition");

    expect("{");

    Node head = {};
    Node *cur = &head;
    while (!consume("}")) {
        cur->next = stmt();
        check_type(cur->next);
        cur = cur->next;
    }

    func->locals = locals;
    func->body = head.next;
    leave_scope();

    return func;
}

// funcparams = ( basetype declarator ( "," basetype declarator | "..." )* )? ")"
void funcparams(Function *func) {
    Var head = {};
    Var *cur = &head;

    int num = 0;
    while (!consume(")")) {
        if (num > 0)
            expect(",");

        if (consume("...")) {
            func->has_varargs = true;
            expect(")");
            break;
        }

        Type *base = basetype();
        Type *type = declarator(base);
        char *name = type->name;

        Var *var = new_params(type, name);
        var->offset = cur->offset + type->size;

        cur->next = var;

        num++;
        cur = cur->next;
    }

    locals = head.next;
    func->params = head.next;
    return;
}

// stmt = "return" expr? ";"
//        | "{" stmt* "}"
//        | "if" "(" expr ")" stmt ( "else" stmt )?
//        | "while" "(" expr ")" stmt
//        | "do" stmt "while" "(" expr ")" ";"
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
    Token *tok;

    if (tok = consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RET;
        node->tok = tok;
        if (consume(";"))
            return node;
        node->lhs = expr();
        expect(";");
        return node;
    }

    if (tok = consume("{")) {
        Node head = {};
        Node *cur = &head;

        enter_scope();

        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->tok = tok;
        while (!consume("}")) {
            cur->next = stmt();
            check_type(cur->next);
            cur = cur->next;
        }
        node->blocks = head.next;

        leave_scope();

        return node;
    }

    if (tok = consume("if")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->tok = tok;
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

    if (tok = consume("while")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        node->tok = tok;
        expect("(");
        node->cond = expr();
        check_type(node->cond);
        expect(")");
        node->then = stmt();
        check_type(node->then);
        return node;
    }

    if (tok = consume("do")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_DO;
        node->tok = tok;
        node->then = stmt();
        check_type(node->then);
        expect("while");
        expect("(");
        node->cond = expr();
        check_type(node->cond);
        expect(")");
        expect(";");
        return node;
    }

    if (tok = consume("for")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        node->tok = tok;
        expect("(");

        enter_scope();
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
        leave_scope();
        check_type(node->then);
        return node;
    }

    if (tok = consume("switch")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_SWITCH;
        node->tok = tok;
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

    if (tok = consume("case")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_CASE;
        node->tok = tok;
        node->val = const_expr();
        expect(":");
        node->lhs = stmt();
        node->next_case = current_switch->next_case;
        current_switch->next_case = node;
        return node;
    }

    if (tok = consume("default")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_CASE;
        node->tok = tok;
        expect(":");
        node->lhs = stmt();
        current_switch->default_case = node;
        return node;
    }

    if (tok = consume("break")) {
        expect(";");
        return new_node_break(tok);
    }

    if (tok = consume("continue")) {
        expect(";");
        return new_node_continue(tok);
    }

    if (tok = consume("goto")) {
        Token *tok = consume_ident();
        node = new_node_goto(tok);
        node->labelname = strndup(tok->str, tok->len);
        expect(";");
        return node;
    }

    if (token->kind == TK_IDENT) {
        Token *tok = consume_ident();
        if (consume(":")) {
            node = new_node_label(tok);
            node->labelname = strndup(tok->str, tok->len);
            node->lhs = stmt();
            return node;
        }
        token = tok;
    }

    if (peek("typedef")) {
        typedefs();
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
void *typedefs() {
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
}

// local variable initializer

typedef struct Designator Designator;
struct Designator {
    Designator *next;
    int idx;     // array
    Member *mem; // struct
};

Node *new_designator_helper(Var *var, Designator *desg, Token *tok) {
    if (!desg)
        return new_node_var(var, tok);

    Node *node = new_designator_helper(var, desg->next, tok);

    if (desg->mem) {
        Node *n = calloc(1, sizeof(Node));
        n->kind = ND_MEMBER;
        n->tok = desg->mem->tok;
        n->lhs = node;
        n->member = desg->mem;
        return n;
    }

    node = new_add(node, new_node_num(desg->idx, tok), tok);
    return new_node_deref(node, tok);
}

Node *new_designator(Var *var, Designator *desg, Node *rhs) {
    Node *lhs = new_designator_helper(var, desg, rhs->tok);
    Node *node = new_node(ND_AS, lhs, rhs, rhs->tok);

    Node *ret = calloc(1, sizeof(Node));
    ret->kind = ND_EXPR_STMT;
    ret->lhs = node;
    ret->tok = rhs->tok;
    return ret;
}

Node *lvar_init_zero(Node *cur, Var *var, Type *type, Designator *desg) {
  if (type->kind == TY_ARR) {
    for (int i = 0; i < type->size_array; i++) {
      Designator desg2 = {desg, i};
      cur = lvar_init_zero(cur, var, type->ptr_to, &desg2);
    }
    return cur;
  }

  cur->next = new_designator(var, desg, new_node_num(0, token));
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
            Node *rhs = new_node_num(tok->str[i], tok);
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

        if (!peek("}")) {
            do {
                Designator desg2 = {desg, i++};
                cur = lvar_initializer_helper(cur, var, type->ptr_to, &desg2);
            } while (!peek_end() && consume(","));
        }

        if (consume(","))
            expect("}");
        else
            expect("}");

        if (type->is_incomplete) {
            type->size = type->ptr_to->size * i;
            type->size_array = i;
            type->is_incomplete = false;
        }

        // padding extra array elements with zero value
        for (; i<type->size_array; i++) {
            Designator desg2 = {desg, i};
            cur = lvar_init_zero(cur, var, type->ptr_to, &desg2);
        }

        return cur;
    }

    if (type->kind == TY_STRUCT) {
        expect("{");
        Member *mem = type->members;

        if (!peek("}")) {
            do {
                Designator desg2 = {desg, 0, mem};
                cur = lvar_initializer_helper(cur, var, mem->type, &desg2);
                mem = mem->next;
            } while (mem && !peek_end() && consume(","));
        }

        if (consume(","))
            expect("}");
        else
            expect("}");

        // set excess struct elements to zero
        for (; mem; mem=mem->next) {
            Designator desg2 = {desg, 0, mem};
            cur = lvar_init_zero(cur, var, mem->type, &desg2);
        }

        return cur;
    }

    bool open_brace = consume("{");
    cur->next = new_designator(var, desg, assign());
    if (open_brace)
        expect("}");
    return cur->next;
}

Node *lvar_initializer(Var *var, Token *tok) {
    Node head = {};
    lvar_initializer_helper(&head, var, var->type, NULL);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->tok = tok;
    node->blocks = head.next;
    return node;
}

// declaration = basetype declarator ( "=" ( lvar_initializer ) )? ( "," declarator ( "=" ( lvar_initializer ) )? )* ";"
Node *declaration() {
    Node head = {};
    Node *cur = &head;

    Token *tok = token;
    bool is_static = consume("static");
    Type *base = basetype();

    int num = 0;
    while (!consume(";")) {
        Token *tok = token;
        if (num > 0)
            expect(",");

        Type *type = declarator(base);

        if (is_static) {
            Var *var = new_gvar(type, type->name, false, is_static);
            if (consume("="))
                var->initializer = gvar_initializer(type);
            else if (type->is_incomplete)
                error_tok(tok, "incomplete type");
            expect(";");

            return new_node_null_expr(tok);
        }

        Var *var = new_lvar(type, type->name);
        var->offset = locals->offset + type->size;

        if (consume("="))
            cur->next = lvar_initializer(var, tok);
        else
            cur->next = new_node_null_expr(tok);

        num++;
        cur = cur->next;
    }

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->tok = tok;
    node->blocks = head.next;

    return node;
}

// is_typename = "int" | "char" | "short" | "long" | "_Bool"
//             | "struct" | "void" | typedef_name | "enum"
bool is_typename() {
    if (peek("int") || peek("char") || peek("short") || peek("long") || peek("_Bool")
        || peek("struct") || peek("void") || find_type(token) || peek("enum") || peek("static"))
        return true;

    return false;
}

// basetype = is_typename | typedef_name
Type *basetype() {
    if (!is_typename())
        error_tok(token, "expected typename");

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
        // *placeholder = *type_suffix(type); => TODO: stage2 でエラー
        memcpy(placeholder, type_suffix(type), sizeof(Type));
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
        // *placeholder = *type_suffix(type); => TODO: stage2 でエラー
        memcpy(placeholder, type_suffix(type), sizeof(Type));
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
        error_tok(token, "incomplete token type");
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
                offset = align(offset, m->type->align);
                m->offset = offset;
                offset += m->type->size;

                if (type->align < m->type->align)
                    type->align = m->type->align;
            }
            type->size = align(offset, type->align);

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
        offset = align(offset, m->type->align);
        m->offset = offset;
        offset += m->type->size;

        if (type->align < m->type->align)
            type->align = m->type->align;
    }
    type->size = align(offset, type->align);

    return type;
}

// struct_member = ( basetype declarator ( "," declarator )* ";" )* "}"
Member *struct_members() {
    Member head = {};
    Member *cur = &head;

    while (!consume("}")) {
        int num = 0;
        Type *base = basetype();
        Token *tok = token;
        while (!consume(";")) {
            if (num > 0)
                expect(",");

            Member *member = calloc(1, sizeof(Member));
            member->type = declarator(base);
            member->tok = tok;
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
                error_tok(token, "unknown tag name");
            if (tsc->tag->type->kind != TY_ENUM)
                error_tok(token, "not enum type");
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

        var->is_enum = true;
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
    node->tok = token;
    return node;
}

// expr = assign ( "," assign )*
Node *expr() {
    Node *node = assign();
    Token *tok;

    while (tok = consume(","))
        node = new_node(ND_COMMA, node, assign(), tok);

    return node;
}

// x op= y => tmp = &x, *tmp = *tmp op y
Node *compound_assign(Node *node) {
    check_type(node->lhs);
    check_type(node->rhs);

    Var *tmp = new_lvar(pointer_to(node->lhs->type), "");
    Token *tok = node->tok;

    Node *e1 = new_node(ND_AS, new_node_var(tmp, tok), new_node_addr(node->lhs, tok), tok);
    Node *e2 = new_node(ND_AS,
                        new_node_deref(new_node_var(tmp, tok), tok),
                        new_node(node->kind,
                                 new_node_deref(new_node_var(tmp, tok), tok),
                                 node->rhs,
                                 tok),
                        tok);

    return new_node(ND_COMMA, e1, e2, tok);
}

// assign = conditional ( "=" assign | "+=" assign | "-=" assign | "*=" assign | "/=" assign | "%=" assign )?
Node *assign() {
    Node *node = conditional();

    Token *tok;
    if (tok = consume("="))
        return new_node(ND_AS, node, assign(), tok);
    if (tok = consume("+="))
        return compound_assign(new_add(node, assign(), tok));
    if (tok = consume("-="))
        return compound_assign(new_sub(node, assign(), tok));
    if (tok = consume("*="))
        return compound_assign(new_node(ND_MUL, node, assign(), tok));
    if (tok = consume("/="))
        return compound_assign(new_node(ND_DIV, node, assign(), tok));
    if (tok = consume("%="))
        return compound_assign(new_node(ND_MOD, node, assign(), tok));

    return node;
}

int const_expr() {
  return eval(conditional());
}

// conditional = logicOr ( "?" expr ":" conditional )?
Node *conditional() {
    Node *node = logicOr();

    Token *tok;
    if (tok = consume("?")) {
        Node *node2 = calloc(1, sizeof(Node));
        node2->kind = ND_COND;
        node2->tok = tok;
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

// logicOr = logicAnd ( "||" logicAnd )*
Node *logicOr() {
    Node *node = logicAnd();

    Token *tok;
    for (;;) {
        if (tok = consume("||"))
            node = new_node(ND_LOGOR, node, logicAnd(), tok);
        else
            return node;
    }
}

// logicAnd = equality ( "&&" equality )*
Node *logicAnd() {
    Node *node = equality();

    Token *tok;
    for (;;) {
        if (tok = consume("&&"))
            node = new_node(ND_LOGAND, node, equality(), tok);
        else
            return node;
    }
}

// equality = relational ( "==" relational | "!=" relational )*
Node *equality() {
    Node *node = relational();

    Token *tok;
    for (;;) {
        if (tok = consume("=="))
            node = new_node(ND_EQ, node, relational(), tok);
        else if (tok = consume("!="))
            node = new_node(ND_NE, node, relational(), tok);
        else
            return node;
    }
}

// relational = add ( ">" add | "<" add | ">=" add | "<=" add )*
Node *relational() {
    Node *node = add();

    Token *tok;
    for (;;) {
        if (tok = consume(">="))
            node = new_node(ND_OM, node, add(), tok);
        else if (tok = consume("<="))
            node = new_node(ND_OL, node, add(), tok);
        else if (tok = consume(">"))
            node = new_node(ND_MT, node, add(), tok);
        else if (tok = consume("<"))
            node = new_node(ND_LT, node, add(), tok);
        else
            return node;
    }
}

// add = mul ( "+" mul | "-" mul )*
Node *add() {
    Node *node = mul();

    Token *tok;
    for (;;) {
        if (tok = consume("+"))
            node = new_add(node, mul(), tok);
        else if (tok = consume("-"))
            node = new_sub(node, mul(), tok);
        else
            return node;
    }
}

// mul = cast ( '*' cast | '/' cast | '%' cast )*
Node *mul() {
    Node *node = cast();

    Token *tok;
    for (;;) {
        if (tok = consume("*"))
            node = new_node(ND_MUL, node, cast(), tok);
        else if (tok = consume("/"))
            node = new_node(ND_DIV, node, cast(), tok);
        else if (tok = consume("%"))
            node = new_node(ND_MOD, node, cast(), tok);
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
            node->tok = tok;
            Type *type = basetype();
            node->type = abstract_declarator(type);
            expect(")");
            if (!consume("{")) {
                node->lhs = cast();
                check_type(node->lhs);
                return node;
            }
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
    Token *tok;
    if (consume("+"))
        return cast();
    if (tok = consume("-"))
        return new_node(ND_SUB, new_node_num(0, tok), cast(), tok);
    if (tok = consume("&"))
        return new_node_addr(cast(), tok);
    if (tok = consume("*"))
        return new_node_deref(cast(), tok);
    if (tok = consume("!"))
        return new_node_not(cast(), tok);

    if (tok = consume("++"))
        return compound_assign(new_add(unary(), new_node_num(1, tok), tok));
    if (tok = consume("--"))
        return compound_assign(new_sub(unary(), new_node_num(1, tok), tok));

    if (consume("sizeof")) {
        Token *tok = token;
        if (consume("(")) {
            if (is_typename()) {
                Type *type = basetype();
                type = abstract_declarator(type);
                expect(")");
                return new_node_num(type->size, tok);
            }
            token = tok; // bring back the token sequense
        }

        Node *node = unary();
        check_type(node);
        return new_node_num(node->type->size, tok);
    }

    return postfix();
}

// x++ => tmp = &x, *tmp = *tmp + 1, *tmp - 1
// x-- => tmp = &x, *tmp = *tmp - 1, *tmp + 1
Node *postop(Node *node, int val, Token *tok) {
    check_type(node);
    Var *tmp = new_lvar(pointer_to(node->type), "");

    Node *e1 = new_node(ND_AS, new_node_var(tmp, tok), new_node_addr(node, tok), tok);
    Node *e2 = new_node(ND_AS,
                        new_node_deref(new_node_var(tmp, tok), tok),
                        new_add(new_node_deref(new_node_var(tmp, tok), tok), new_node_num(val, tok), tok),
                        tok);
    Node *e3 = new_add(new_node_deref(new_node_var(tmp, tok), tok), new_node_num(val*-1, tok), tok);

    return new_node(ND_COMMA, e1, new_node(ND_COMMA, e2, e3, tok), tok);
}

// postfix = compound_literal
//         | primary ( ( "[" expr "]" ) | ( "." ident ) | ( "->" ident ) | "++" | "--" )*
Node *postfix() {
    Node *node = compound_literal();
    if (node)
        return node;

    node = primary();
    Token *tok;
    for (;;) {
        if (tok = consume("[")) {
            Node *idx = expr();
            expect("]");
            node = new_node_deref(new_add(node, idx, tok), tok);
            continue;
        }

        if (tok = consume(".")) {
            // access struct member
            node = struct_ref(node);
            continue;
        }

        if (tok = consume("->")) {
            // b->n => (*b).n
            node = struct_ref(new_node_deref(node, tok));
            continue;
        }

        if (tok = consume("++")) {
            node = postop(node, 1, tok);
            continue;
        }

        if (tok = consume("--")) {
            node = postop(node, -1, tok);
            continue;
        }

        return node;
    }
}

// compound_literal = '(' typename ')' '{' ( gvar_initializer | lvar_initializer ) '}'
Node *compound_literal() {
    Token *tok = token;
    if (!consume("(") || !is_typename()) {
        token = tok;
        return NULL;
    }

    Type *type = basetype();
    type = abstract_declarator(type);

    expect(")");
    if (!peek("{")) {
        token = tok;
        return NULL;
    }

    if (scope_depth == 0) {
        Var *var = new_gvar(type, new_label(), false, true);
        var->initializer = gvar_initializer(type);
        return new_node_var(var, tok);
    }

    Var *var = new_lvar(type, new_label());
    Node *node = new_node_var(var, tok);
    node->init = lvar_initializer(var, tok);
    return node;
}

Node *struct_ref(Node *node) {
    check_type(node);
    if (node->type->kind != TY_STRUCT)
        error_tok(node->tok, "not struct");

    Token *tok = consume_ident();
    Member *mem = get_struct_member(node->type, strndup(tok->str, tok->len));

    Node *n = calloc(1, sizeof(Node));
    n->kind = ND_MEMBER;
    n->tok = tok;
    n->lhs = node;
    n->member = mem;

    return n;
}

Member *get_struct_member(Type *type, char *name) {
    for (Member *m=type->members; m; m=m->next) {
        if (strcmp(m->name, name) == 0)
            return m;
    }

    error_tok(token, "no such member");
}

// primary = '(' '{' stmt ( stmt )* '}' ')' // last stmt should be expr_stmt
//         | '(' expr ')'
//         | ident ( "(" funcargs* )?
//         | num | '"' str '"'
Node *primary() {
    Token *tok;
    if (tok = consume("(")) {
        if (consume("{")) {
            enter_scope();
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_STMT_EXPR;
            node->tok = tok;
            node->blocks = stmt();
            Node *cur = node->blocks;

            while (!consume("}")) {
                cur->next = stmt();
                cur = cur->next;
            }
            expect(")");
            leave_scope();

            if (cur->kind != ND_EXPR_STMT)
                error_tok(tok, "statement expression returning void is not supported");
            memcpy(cur, cur->lhs, sizeof(Node));
            return node;
        } else {
            Node *node = expr();
            expect(")");
            return node;
        }
    }

    tok = consume_ident();
    if (tok) {
        if (consume("(")) {
            Node *node = new_node_func(strndup(tok->str, tok->len), funcargs(), tok);
            check_type(node);

            Var *var;
            if (var = find_var(tok)) {
                if (var->type->kind != TY_FUNC)
                    error_tok(tok, "not a function");
                node->type = var->type->return_type;
            } else if (!strcmp(node->funcname, "__builtin_va_start")) {
                node->type = void_type;
            } else {
                // implicit declaration of a function
                node->type = int_type;
            }
            return node;
        } else {
            Var *var;
            if (var = find_var(tok)) {
                if (var->is_enum)
                    return new_node_num(var->enum_val, tok);
                return new_node_var(var, tok);
            }
            error_tok(tok, "undefined variable");
        }
    }

    tok = consume_str();
    if (tok) {
        Var *var = new_gvar(array_of(char_type, tok->strlen), new_label(), false, true);
        var->initializer = new_init_string(tok);

        return new_node_var(var, tok);
    }

    return new_node_num(expect_number(), token);
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
