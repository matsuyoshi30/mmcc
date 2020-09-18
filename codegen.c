#include "mmcc.h"

/// Code generator

static char *funcname;
static int labels = 1;
static int brkseq;
static int contseq;
static char *argRegs1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argRegs2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *argRegs4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argRegs8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_expr(Node *node);
void gen_stmt(Node *node);

void gen_lval(Node *node) {
    if (node->kind == ND_LV) {
        if (node->init)
            gen_stmt(node->init);

        if (node->var->is_local)
            printf("  lea rax, [rbp-%d] # %s\n", node->var->offset, node->var->name);
        else
            printf("  lea rax, %s[rip]\n", node->var->name);
        printf("  push rax\n");
        return;
    } else if (node->kind == ND_DEREF) {
        gen_expr(node->lhs);
        return;
    } else if (node->kind == ND_MEMBER) {
        gen_lval(node->lhs);
        printf("  pop rax\n");
        printf("  add rax, %d\n", node->member->offset);
        printf("  push rax\n");
        return;
    }

    error("The left value of the assignment is not a variable.");
}

void load(Type *type) {
    printf("  pop rax\n");
    if (type->size == 1)
        printf("  movsx rax, byte ptr [rax]\n");
    else if (type->size == 2)
        printf("  movsx rax, word ptr [rax]\n");
    else if (type->size == 4)
        printf("  movsx rax, dword ptr [rax]\n");
    else
        printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
}

void typecast(Type *type) {
    printf("  pop rax\n");

    if (type->kind == TY_BOOL) {
        printf("  cmp rax, 0\n");
        printf("  setne al\n");
        printf("  movzx rax, al\n");
        printf("  push rax\n");
        return;
    }

    if (type->size == 1)
        printf("  movsx rax, al\n");
    else if (type->size == 2)
        printf("  movsx rax, ax\n");
    else if (type->size == 4)
        printf("  movsx rax, eax\n");
    printf("  push rax\n");
    return;
}

void gen_expr(Node *node) {
    printf("  .loc 1 %d\n", node->tok->line_no);

    switch (node->kind) {
    case ND_NUM:
        if (node->val == (int)node->val) {
            printf("  push %ld\n", node->val);
        } else {
            printf("  movabs rax, %ld\n", node->val);
            printf("  push rax\n");
        }
        return;
    case ND_LV:
        if (node->init)
            gen_stmt(node->init);
        gen_lval(node);
        if (node->type->kind != TY_ARR)
            load(node->type);
        return;
    case ND_MEMBER:
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
    case ND_CAST:
        gen_expr(node->lhs);
        typecast(node->type);
        return;
    case ND_NOT:
        gen_expr(node->lhs);
        // lhs: true     -> al: false
        // lhs: false(0) -> al: true
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        printf("  push rax\n");
        return;
    case ND_COND: {
        int seq = labels++;
        gen_expr(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lelse.%d\n", seq);
        gen_expr(node->then);
        printf("  jmp .Lend.%d\n", seq);
        printf(".Lelse.%d:\n", seq);
        gen_expr(node->els);
        printf(".Lend.%d:\n", seq);
        return;
    }
    case ND_LOGOR: {
        int seq = labels++;
        gen_expr(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .Ltrue.%d\n", seq);
        gen_expr(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  jne .Ltrue.%d\n", seq);
        printf("  push 0\n"); // false
        printf("  jmp .Lend.%d\n", seq);
        printf(".Ltrue.%d:\n", seq);
        printf("  push 1\n"); // true
        printf(".Lend.%d:\n", seq);
        return;
    }
    case ND_LOGAND: {
        int seq = labels++;
        gen_expr(node->lhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lfalse.%d\n", seq);
        gen_expr(node->rhs);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lfalse.%d\n", seq);
        printf("  push 1\n"); // true
        printf("  jmp .Lend.%d\n", seq);
        printf(".Lfalse.%d:\n", seq);
        printf("  push 0\n"); // false
        printf(".Lend.%d:\n", seq);
        return;
    }
    case ND_FUNC: {
        int num_of_args = 0;
        for (Node *arg=node->args; arg; arg=arg->next) {
            gen_expr(arg);
            num_of_args++;
        }

        for (int i=num_of_args-1; i>=0; i--)
            printf("  pop %s\n", argRegs8[i]);

        int seq = labels++;
        printf("  mov rax, rsp\n");
        printf("  and rax, 15\n"); // rax & 0b1111
        printf("  jnz .Lcall.%d\n", seq);
        printf("  mov al, 0\n");
        printf("  call %s\n", node->funcname);
        printf("  jmp .Lend.%d\n", seq);
        // align
        printf(".Lcall.%d:\n", seq);
        printf("  sub rsp, 8\n");
        printf("  mov al, 0\n");
        printf("  call %s\n", node->funcname);
        printf("  add rsp, 8\n");

        printf(".Lend.%d:\n", seq);
        if (node->type->kind == TY_BOOL) {
            printf("  movzb rax, al\n");
        }
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

        if (node->type->kind == TY_BOOL) {
            printf("  cmp rdi, 0\n");
            printf("  setne dil\n");
            printf("  movzb rdi, dil\n");
        }

        if (node->type->size == 1)
            printf("  mov [rax], dil\n"); // store value from dil into the address in rax
        else if (node->type->size == 2)
            printf("  mov [rax], di\n");  // store value from di into the address in rax
        else if (node->type->size == 4)
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
    case ND_NULL_EXPR:
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
    case ND_MOD:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        printf("  mov rax, rdx\n");
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
    printf("  .loc 1 %d\n", node->tok->line_no);

    switch (node->kind) {
    case ND_RET:
        if (node->lhs) {
            gen_expr(node->lhs);
            printf("  pop rax\n");
        }
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
            printf("  je .Lelse.%d\n", seq);
            gen_stmt(node->then);
            printf("  jmp .Lend.%d\n", seq);
            printf(".Lelse.%d:\n", seq);
            gen_stmt(node->els);
            printf(".Lend.%d:\n", seq);
        } else {
            printf("  je .Lend.%d\n", seq);
            gen_stmt(node->then);
            printf(".Lend.%d:\n", seq);
        }
        return;
    }
    case ND_DO: {
        int seq = labels++;
        int brk = brkseq;
        int cont = contseq;
        brkseq = seq;
        contseq = seq;

        printf(".Lbegin.%d:\n", seq);
        gen_stmt(node->then);
        if (node->cond) {
            gen_expr(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lbreak.%d\n", seq);
        }
        printf(".Lcontinue.%d:\n", seq);
        printf("  jmp .Lbegin.%d\n", seq);
        printf(".Lbreak.%d:\n", seq);

        brkseq = brk;
        contseq = cont;
        return;
    }
    case ND_FOR: {
        int seq = labels++;
        int brk = brkseq;
        int cont = contseq;
        brkseq = seq;
        contseq = seq;

        if (node->preop)
            gen_stmt(node->preop);
        printf(".Lbegin.%d:\n", seq);
        if (node->cond) {
            gen_expr(node->cond);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lbreak.%d\n", seq);
        }
        gen_stmt(node->then);
        printf(".Lcontinue.%d:\n", seq);
        if (node->postop)
            gen_stmt(node->postop);
        printf("  jmp .Lbegin.%d\n", seq);
        printf(".Lbreak.%d:\n", seq);

        brkseq = brk;
        contseq = cont;
        return;
    }
    case ND_SWITCH: {
        int seq = labels++;
        int brk = brkseq;
        brkseq = seq;

        gen_expr(node->cond);
        printf("  pop rax\n");

        for (Node *n=node->next_case; n; n=n->next_case) {
            n->case_label = labels++;
            printf("  cmp rax, %ld\n", n->val);
            printf("  je .Lcase.%d\n", n->case_label);
        }

        if (node->default_case) {
            node->default_case->case_label = labels++;
            printf("  jmp .Lcase.%d\n", node->default_case->case_label);
        }

        printf("  jmp .Lbreak.%d\n", seq);
        gen_stmt(node->then);
        printf(".Lbreak.%d:\n", seq);

        brkseq = brk;
        return;
    }
    case ND_CASE:
        printf(".Lcase.%d:\n", node->case_label);
        gen_stmt(node->lhs);
        return;
    case ND_BREAK:
        printf("  jmp .Lbreak.%d\n", brkseq);
        return;
    case ND_CONT:
        printf("  jmp .Lcontinue.%d\n", contseq);
        return;
    case ND_GOTO:
        printf("  jmp .L.%s\n", node->labelname);
        return;
    case ND_LABEL:
        printf(".L.%s:\n", node->labelname);
        gen_stmt(node->lhs);
        return;
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
    if (n == 0)
        return 0;

    if (n < align)
        return align;
    while (n%align)
        n++;
    return n;
}

void emit_data(Program *prog) {
    for (Var *global=prog->globals; global->next; global=global->next) {
        if (!global->is_static)
            printf(".global %s\n", global->name);
    }

    printf(".bss\n");
    for (Var *global=prog->globals; global->next; global=global->next) {
        if (global->initializer)
            continue;

        printf("%s:\n", global->name);
        printf("  .zero %d\n", global->type->size);
    }

    printf(".data\n");
    for (Var *global=prog->globals; global->next; global=global->next) {
        if (!global->initializer)
            continue;

        printf("%s:\n", global->name);

        for (Initializer *init=global->initializer; init; init=init->next) {
            if (init->label)
                printf("  .quad %s\n", init->label); // another variable's address
            else if (init->size == 1)
                printf("  .byte %ld # %c\n", init->val, (char)(init->val==10 ? 0 : init->val));
            else
                printf("  .%dbyte %ld\n", init->size, init->val);
        }
    }
}

void emit_text(Program *prog) {
    printf(".text\n");
    for (Function *func=prog->code; func; func=func->next) {
        funcname = func->name;
        if (!func->is_static)
            printf(".global %s\n", func->name);
        printf("%s:\n", funcname);

        int stackSize = 0;
        for (Var *var=func->locals; var; var=var->next) {
            stackSize = align(stackSize, var->type->align);
            stackSize += var->type->size;
            var->offset = stackSize;
        }
        stackSize = align(stackSize, 8);

        // prologue
        printf("  push rbp\n");
        printf("  mov rbp, rsp\n");
        printf("  sub rsp, %d\n", stackSize);

        int i = 0;
        for (Var *param=func->params; param; param=param->next) {
            if (param->type->size == 1)
                printf("  mov [rbp-%d], %s # %s\n", param->offset, argRegs1[i++], param->name);
            else if (param->type->size == 2)
                printf("  mov [rbp-%d], %s # %s\n", param->offset, argRegs2[i++], param->name);
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

void codegen(Program *prog) {
    printf(".intel_syntax noprefix\n");
    emit_data(prog);
    emit_text(prog);
}
