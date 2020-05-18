#include "mmcc.h"

// Code generator

static char *funcname;
static int labels = 1;
static char *argRegs1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argRegs4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argRegs8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_expr(Node *node);
void gen_stmt(Node *node);

void gen_lval(Node *node) {
    if (node->kind == ND_LV) {
        if (node->var->is_local)
            printf("  lea rax, [rbp-%d] # %s\n", node->var->offset, node->var->name);
        else if (node->var->str)
            printf("  lea rax, .LC%d\n", node->var->lc);
        else
            printf("  lea rax, %s[rip]\n", node->var->name);
        printf("  push rax\n");
        return;
    } else if (node->kind == ND_DEREF) {
        gen_expr(node->lhs);
        return;
    }

    error("The left value of the assignment is not a variable.");
}

void load(Type *type) {
    printf("  pop rax\n");
    if (type->size == 1)
        printf("  movsx rax, byte ptr [rax]\n");
    else if (type->size == 4)
        printf("  movsxd rax, dword ptr [rax]\n");
    else
        printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
}

void gen_expr(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LV:
        gen_lval(node);
        if (node->type->kind != TY_ARR)
            load(node->type);
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    case ND_DEREF:
        gen_expr(node->lhs);
        if (node->type->kind != TY_ARR)
            load(node->type);
        return;
    case ND_FUNC: {
        int num_of_args = 0;
        for (Node *arg=node->args; arg; arg=arg->next) {
            gen_expr(arg);
            num_of_args++;
        }

        for (int i=num_of_args-1; i>=0; i--)
            printf("  pop %s\n", argRegs8[i]);
        printf("  mov al, 0\n");
        printf("  call %s\n", node->funcname);
        printf("  push rax\n");
        return;
    }
    case ND_AS:
        if (node->lhs->type->kind == TY_ARR)
            error("not an left value\n");
        gen_lval(node->lhs);
        gen_expr(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        if (node->type->size == 4)
            printf("  mov [rax], edi\n"); // store value from edi into the address in rax
        else
            printf("  mov [rax], rdi\n"); // store value from rdi into the address in rax
        printf("  push rdi\n");
        return;
    case ND_STMT_EXPR:
        for (Node *block=node->blocks; block; block=block->next)
            gen_stmt(block);
        printf("  push rax\n");
        return;
    case ND_COMMA:
        gen_expr(node->lhs);
        printf("  pop rax\n");
        gen_expr(node->rhs);
        return;
    }

    gen_expr(node->lhs);
    gen_expr(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_OM:
        printf("  cmp rdi, rax\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_OL:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_MT:
        printf("  cmp rdi, rax\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    default:
        error("unable generate code\n");
    }

    printf("  push rax\n");
}

void gen_stmt(Node *node) {
    switch (node->kind) {
    case ND_RET:
        gen_expr(node->lhs);
        printf("  pop rax\n");
        printf("  jmp .Lreturn.%s\n", funcname);
        return;
    case ND_BLOCK:
        for (Node *block=node->blocks; block; block=block->next)
            gen_stmt(block);
        return;
    case ND_IF: {
        int seq = labels++;
        gen_expr(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->els) {
            printf("  je .Lelse%03d\n", seq);
            gen_stmt(node->then);
            printf("  jmp .Lend%03d\n", seq);
            printf(".Lelse%03d:\n", seq);
            gen_stmt(node->els);
            printf(".Lend%03d:\n", seq);
        } else {
            printf("  je .Lend%03d\n", seq);
            gen_stmt(node->then);
            printf(".Lend%03d:\n", seq);
        }
        return;
    }
    case ND_WHILE: {
        int seq = labels++;
        printf(".Lbegin%03d:\n", seq);
        gen_expr(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", seq);
        gen_stmt(node->then);
        printf("  jmp .Lbegin%03d\n", seq);
        printf(".Lend%03d:\n", seq);
        return;
    }
    case ND_FOR: {
        int seq = labels++;
        if (node->preop)
            gen_stmt(node->preop);
        printf(".Lbegin%03d:\n", seq);
        if (node->cond) {
            gen_expr(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%03d\n", seq);
        }
        gen_stmt(node->then);
        if (node->postop)
            gen_stmt(node->postop);
        printf("  jmp .Lbegin%03d\n", seq);
        printf(".Lend%03d:\n", seq);
        return;
    }
    case ND_COMMA:
        gen_expr(node->lhs);
        printf("  pop rax\n");
        gen_expr(node->rhs);
        return;
    case ND_EXPR_STMT:
        gen_expr(node->lhs);
        printf("  pop rax\n");
        return;
    }
}

int align(int n, int align) {
    if (n < align)
        return align;
    while (n%align)
        n++;
    return n;
}

void codegen() {
    printf(".intel_syntax noprefix\n");

    printf(".data\n");
    for (Var *str=strs; str; str=str->next) {
        printf(".LC%d:\n", str->lc);
        for (int i=0; i<strlen(str->str); i++)
            printf("  .byte %d # %c\n", str->str[i], str->str[i]==10 ? 0 : str->str[i]);
        printf("  .byte 0\n");
    }
    for (Var *global=globals; global->next; global=global->next) {
        printf("%s:\n", global->name);
        printf("  .zero %d\n", global->type->size);
    }

    printf(".text\n");
    for (Function *func=code; func; func=func->next) {
        funcname = func->name;
        printf(".global %s\n", func->name);
        printf("%s:\n", funcname);

        int stackSize = 0;
        for (Var *var=func->locals; var; var=var->next) {
            stackSize += var->type->size;
            var->offset = stackSize;
        }
        stackSize = align(stackSize, 16);

        // prologue
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", stackSize);

        int i = 0;
        for (Var *param=func->params; param; param=param->next) {
            if (param->type->size == 1)
                printf("  mov [rbp-%d], %s # %s\n", param->offset, argRegs1[i++], param->name);
            else if (param->type->size == 4)
                printf("  mov [rbp-%d], %s # %s\n", param->offset, argRegs4[i++], param->name);
            else
                printf("  mov [rbp-%d], %s # %s\n", param->offset, argRegs8[i++], param->name);
        }

        for (Node *node=func->body; node; node=node->next)
            gen_stmt(node);

        // epilogue
        printf(".Lreturn.%s:\n", funcname);
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}
