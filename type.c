#include "mmcc.h"

Type *void_type = &(Type){TY_VOID, 1, 1};
Type *char_type = &(Type){TY_CHAR, 1, 1};
Type *short_type = &(Type){TY_SHORT, 2, 2};
Type *int_type = &(Type){TY_INT, 4, 4};
Type *long_type = &(Type){TY_LONG, 8, 8};
Type *bool_type = &(Type){TY_BOOL, 1, 1};
Type *enum_type = &(Type){TY_ENUM, 4, 4};

bool is_integer(Type *type) {
    return type->kind == TY_INT || type->kind == TY_CHAR
        || type->kind == TY_SHORT || type->kind == TY_LONG
        || type->kind == TY_BOOL;
}

Type *pointer_to(Type *ty) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_PTR;
    type->size = 8;
    type->align = 8;
    type->ptr_to = ty;
    return type;
}

Type *array_of(Type *ty, int n) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_ARR;
    type->size = ty->size * n;
    type->align = ty->align;
    type->ptr_to = ty;
    type->size_array = n;
    return type;
}

Type *func_type(Type *return_type) {
    Type *type = calloc(1, sizeof(Type));
    type->kind = TY_FUNC;
    type->size = 1;
    type->align = 1;
    type->return_type = return_type;
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

    check_type(node->init);

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
    case ND_EQ:
    case ND_NE:
    case ND_MT:
    case ND_LT:
    case ND_OM:
    case ND_OL:
        node->type = int_type;
        return;
    case ND_NOT:
    case ND_LOGOR:
    case ND_LOGAND:
        node->type = int_type;
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
            error_tok(node->tok, "invalid pointer dereference");
        node->type = node->lhs->type->ptr_to;
        return;
    case ND_STMT_EXPR: {
        Node *block = node->blocks;
        while (block->next)
            block = block->next;
        node->type = block->type;
        return;
    }
    case ND_EXPR_STMT:
        node->type = node->lhs->type;
        return;
    case ND_COMMA:
        node->type = node->rhs->type;
        return;
    case ND_MEMBER:
        node->type = node->member->type;
        return;
    case ND_NUM:
        node->type = (node->val == (int)node->val) ? int_type : long_type;
        return;
    }
}
