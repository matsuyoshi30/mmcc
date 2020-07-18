#include "mmcc.h"

void nop() {}

int main(int argc, char **argv) {
    if (argc != 2)
        error("wrong the number of arguments\n");
    char *filename = argv[1];
    char *user_input = read_file(filename);

    tokenize(filename, user_input); // tokenize input
    program();  // parse tokens into AST
    codegen();  // emit code from AST

    return 0;
}
