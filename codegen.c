#include "mmcc.h"

// Code generator

static int labels = 1;
static char *argRegs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen(Node *node);

void gen_lval(Node *node) {
    if (node->kind == ND_LV) {
        printf("  mov rax, rbp\n");
        printf("  sub rax, %d\n", node->lvar->offset);
        printf("  push rax\n");
        return;
    } else if (node->kind == ND_DEREF) {
        gen(node->lhs);
        return;
    }

    error("The left value of the assignment is not a variable.");
}

void load() {
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_RET:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_BLOCK:
        for (Node *block=node->blocks; block; block=block->next)
            gen(block);
        return;
    case ND_IF: {
        int seq = labels++;
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->els) {
            printf("  je .Lelse%03d\n", seq);
            gen(node->then);
            printf("  jmp .Lend%03d\n", seq);
            printf(".Lelse%03d:\n", seq);
            gen(node->els);
            printf(".Lend%03d:\n", seq);
        } else {
            printf("  je .Lend%03d\n", seq);
            gen(node->then);
            printf(".Lend%03d:\n", seq);
        }
        return;
    }
    case ND_WHILE: {
        int seq = labels++;
        printf(".Lbegin%03d:\n", seq);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", seq);
        gen(node->then);
        printf("  jmp .Lbegin%03d\n", seq);
        printf(".Lend%03d:\n", seq);
        return;
    }
    case ND_FOR: {
        int seq = labels++;
        if (node->preop)
            gen(node->preop);
        printf(".Lbegin%03d:\n", seq);
        if (node->cond)
            gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", seq);
        gen(node->then);
        if (node->postop)
            gen(node->postop);
        printf("  jmp .Lbegin%03d\n", seq);
        printf(".Lend%03d:\n", seq);
        return;
    }
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_FUNC: {
        int num_of_args = 0;
        for (Node *arg=node->args; arg; arg=arg->next) {
            gen(arg);
            num_of_args++;
        }

        for (int i=num_of_args-1; i>=0; i--)
            printf("  pop %s\n", argRegs[i]);
        printf("  call %s\n", node->funcname);
        printf("  push rax\n");
        return;
    }
    case ND_LV:
        gen_lval(node);
        load();
        return;
    case ND_AS:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n"); // store value from rdi into the address in rax
        printf("  push rdi\n");
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    case ND_DEREF:
        gen(node->lhs);
        load();
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

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

int align(int n, int align) {
    if (n < align)
        return align;
    while (n%align)
        n++;
    return n;
}

void codegen() {
    printf(".intel_syntax noprefix\n");

    for (Function *func=code; func; func=func->next) {
        printf(".global %s\n", func->name);
        printf("%s:\n", func->name);

        int stackSize = 0;
        for (LVar *lvar=func->locals; lvar; lvar=lvar->next)
            stackSize += 8;
        stackSize = align(stackSize, 16);

        // prologue
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", stackSize);

        int i = 0;
        for (LVar *param=func->params; param; param=param->next) {
            printf("  mov [rbp-%d], %s\n", param->offset, argRegs[i++]);
        }

        for (Node *node=func->body; node; node=node->next) {
            gen(node);
            printf("  pop rax\n");
        }

        // epilogue
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
    }
}
