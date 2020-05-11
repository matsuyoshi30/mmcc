int alloc4(int *p, int a, int b, int c, int d) {
    p = malloc(16);
    p[0] = a;
    p[1] = b;
    p[2] = c;
    p[3] = d;
}

int main() {
    int x;
    alloc4(&x, 1, 2, 4, 8);
    int *y = &x;
    return *(y+2);
}
