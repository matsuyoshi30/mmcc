#include "mmcc.h"

char *filename;
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

    char *line = loc;
    while (user_input < line && line[-1] != '\n')
        line--;

    char *end = loc;
    while (*end != '\n')
        end++;

    int line_num = 1;
    for (char *p=user_input; p<line; p++)
        if (*p == '\n')
            line_num++;

    int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    exit(1);
}

char *read_file(char *path) {
    FILE *fp;

    if (strcmp(path, "-") == 0) {
        fp = stdin;
    } else {
        fp = fopen(path, "r");
        if (!fp)
            error("cannot open %s: %s\n", path, strerror(errno));
    }

    size_t size;
    if (fp != stdin) {
        if (fseek(fp, 0, SEEK_END) == -1)
            error("%s: fseek: %s\n", path, strerror(errno));
        size = ftell(fp);
        if (fseek(fp, 0, SEEK_SET) == -1)
            error("%s: fseek: %s\n", path, strerror(errno));
    } else {
        size = 4096;
    }

    char *buf = calloc(1, size+2);
    fread(buf, size, 1, fp);

    if (size == 0 || buf[size-1] != '\n')
        buf[size++] = '\n';
    buf[size] = '\0';

    if (fp != stdin)
        fclose(fp);

    return buf;
}

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
