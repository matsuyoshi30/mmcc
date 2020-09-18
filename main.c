#include "mmcc.h"

void nop() {}

int main(int argc, char **argv) {
    if (argc != 2)
        error("wrong the number of arguments\n");
    char *filename = argv[1];
    char *user_input = read_file(filename);

    printf(".file 1 \"%s\"\n", argv[1]);

    tokenize(filename, user_input); // tokenize input
    Program *prog = program();  // parse tokens into AST
    codegen(prog);  // emit code from AST

    return 0;
}
