#include "mmcc.h"

char *filename;
char *user_input;

int main(int argc, char **argv) {
    if (argc != 2)
        error("wrong the number of arguments\n");
    filename = argv[1];
    user_input = read_file(filename);

    tokenize(); // tokenize input
    program();  // parse tokens into AST
    codegen();  // emit code from AST

    return 0;
}
