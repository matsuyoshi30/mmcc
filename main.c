#include "mmcc.h"

char *user_input;

void error(char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);

    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list arg;
    va_start(arg, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2)
        error("wrong the number of arguments\n");

    user_input = argv[1];

    tokenize(); // tokenize input
    program();  // parse tokens into AST
    codegen();  // emit code from AST

    return 0;
}
