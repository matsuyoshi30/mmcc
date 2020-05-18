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

void nop() {}

char *read_file(char *path) {
    FILE *fp;

    if (strcmp(path, "-") == 0) {
        fp = stdin;
    } else {
        fp = fopen(path, "r");
        if (!fp)
            error("cannot open %s: %s\n", path, strerror(errno));
    }

    int buflen = 4096;
    int nread = 0;
    char *buf = malloc(buflen);

    // Read the entire file.
    for (;;) {
        int end = buflen - 2; // extra 2 bytes for the trailing "\n\0"
        int n = fread(buf + nread, 1, end - nread, fp);
        if (n == 0)
            break;
        nread += n;
        if (nread == end) {
            buflen *= 2;
            buf = realloc(buf, buflen);
        }
    }

    if (fp != stdin)
        fclose(fp);

    if (nread == 0 || buf[nread-1] != '\n')
        buf[nread++] = '\n';
    buf[nread] = '\0';

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
