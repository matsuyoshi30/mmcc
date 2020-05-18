int assert(int expected, int actual, char *code) {
    if (expected == actual) {
        printf("%s => %d\n", code, actual);
    } else {
        printf("%s => %d expected, but got %d\n", code, expected, actual);
        exit(1);
    }

    return 0;
}

int testFunc1() {
    return 5;
}

int testFunc2(int num1, int num2) {
    return num1 + num2;
}

int testFunc3(int a1, int a2, int a3, int a4, int a5, int a6) {
    return a1 + a2 + a3 + a4 + a5 + a6;
}

int fib(int num) {
    if (num <= 2)
        return 1;
    return fib(num-1) + fib(num-2);
}

int g1;
int g2[4];
int *g3;

int main() {
    assert(0, 0, "0");
    assert(42, 42, "42");

    assert(4, 5+1-2, "5+1-2");
    assert(15, 5*(9-6), "5*(9-6)");
    assert(4, (3+5)/2, "(3+5)/2");

    assert(10, -10+20, "-10+20");
    assert(15, -3*-5, "-3*-5");
    assert(20, - - +20, "- - +20");

    assert(1, 3==3, "3==3");
    assert(0, 4!=4, "4!=4");
    assert(1, 5>3, "5>3");
    assert(0, 6>=9, "6>=9");
    assert(1, 4<5, "4<5");
    assert(0, 6<=3, "6<=3");

    assert(5, ({ int a; a=5; a; }), "{ int a; a=5; a; }");
    assert(1, ({ int c=5>3; c; }), "{ int c=5>3; c; }");
    assert(22, ({ int a; int b; a=13; b=a+9; b; }), "{ int a; int b; a=13; b=a+9; b; }");

    assert(10, ({ int x; if (5>3) { x=10; } else { x=20; } x; }), "{ int x; if (5>3) { 10; } x; }");
    assert(20, ({ int x; if (5<3) { x=10; } else { x=20; } x; }), "{ int x; if (5<3) { 10; } else { 20; } x; }");
    assert(30, ({ int x; int foo=5; if (5<3) { x=10; } else { if (foo==4) { x=20; } else { x=30; } } x; }), "{ inx x; int foo=5; if (5<3) { 10; } else { if (foo==4) { 20; } else { 30; } } x; }");
    assert(40, ({ int x; int foo=5; if (5>3) { if (foo==5) { x=40; } else { x=20; } } else { x=30; } x; }), "{ int x; int foo=5; if (5>3) { if (foo==5) { 40; } else { 20; } } else { 30; } x; }");

    assert(5, ({ int i=1; while (i<5) { i=i+1; } i; }), "{ int i=1; while (i<5) { i=i+1; } i; }");
    assert(10, ({ int x=0; for (int i=0; i<10; i=i+1) { x=x+1; } x; }), "{ int x=0; for (int i=0; i<10; i=i+1) { x=x+1; } x; }");
    assert(55, ({ int j=0; for (int i=0; i<=10; i=i+1) { j=i+j; } j; }), "{ int j=0; for (int i=0; i<=10; i=i+1) { j=i+j; } j; }");
    assert(100, ({ int a=0; for (int i=0; i<10; i=i+1) { if (i==5) { a=100; } } a; }), "{ int a=0; for (int i=0; i<10; i=i+1) { if (i==5) { a=100; } } a; }");

    assert(6, ({ int a; int b; int c; if (5>3) { a=3; b=2; c=a*b; } c; }), "{ int a; int b; int c; if (5>3) { a=3; b=2; c=a*b; } c; }");
    assert(100, ({ int ret=0; for (int i=0; i<10; i=i+1) { int j=0; while (j<10) { ret=ret+1; j=j+1; } } ret; }), "{ int ret=0; for (int i=0; i<10; i=i+1) { int j=0; while (j<10) { ret=ret+1; j=j+1; } } ret; }");

    assert(5, testFunc1(), "{ testFunc1(); }");
    assert(3, testFunc2(1, 2), "{ testFunc2(1, 2); }");
    assert(21, testFunc3(1, 2, 3, 4, 5, 6), "{ testFunc3(1, 2, 3, 4, 5, 6); }");
    assert(8, fib(6), "{ fib(6); }");

    assert(3, ({ int x=3; *&x; }), "{ int x=3; *&x; }");
    assert(3, ({ int x=3; int *y=&x; int **z=&y; **z; }), "{ int x=3; int *y=&x; int **z=&y; **z; }");
    assert(3, ({ int x; int *y=&x; *y=3; x; }), "{ int x; int *y=&x; *y=3; x; }");

    assert(5, ({ int x=3; int y=5; *(&x+1); }), "{ int x=3; int y=5; *(&x+1); }");
    assert(3, ({ int x=3; int y=5; int *z=&y-1; *z; }), "{ int x=3; int y=5; int *z=&y-1; *z; }");
    assert(3, ({ int x=3; int y=5; *(&y-1); }), "{ int x=3; int y=5; *(&y-1); }");
    assert(7, ({ int x=3; int y=5; *(&x+1)=7; y; }), "{ int x=3; int y=5; *(&x+1)=7; y; }");
    assert(7, ({ int x=3; int y=5; *(&y-1)=7; x; }), "{ int x=3; int y=5; *(&y-1)=7; x; }");

    assert(30, ({ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=p+2; *q; }), "{ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=p+2; *q; }");
    assert(40, ({ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=3+p; *q; }), "{ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=3+p; *q; }");
    assert(10, ({ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=p+2; *(q-2); }), "{ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=p+2; *(q-2); }");
    assert(20, ({ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=3+p; *(q-2); }), "{ int *p; int inttype; int *p = malloc(sizeof(inttype)*4); for (int i=1; i<=4; i=i+1) p[i-1] = i*10;int *q=3+p; *(q-2); }");

    assert(4, ({ int x; sizeof(x); }), "{ int x; sizeof(x); }");
    assert(8, ({ int *y; sizeof(y); }), "{ int *y; sizeof(y); }");
    assert(4, ({ sizeof(1); }), "{ sizeof(1); }");
    assert(4, ({ sizeof(sizeof(1)); }), "{ sizeof(sizeof(1)); }");
    assert(4, ({ int x; sizeof(x+3); }), "{ int x; sizeof(x+3); }");
    assert(8, ({ int *y; sizeof(y+3); }), "{ int *y; sizeof(y+3); }");
    assert(4, ({ int *y; sizeof(*y); }), "{ int *y; sizeof(*y); }");
    assert(20, ({ int x[5]; sizeof(x); }), "{ int x[5]; sizeof(x); }");
    assert(16, ({ int x[2][2]; sizeof(x); }), "{ int x[2][2]; sizeof(x); }");

    assert(3, ({ int x[2]; int *y=&x; *y=3; *x; }), "{ int x[2]; int *y=&x; *y=3; *x; }");
    assert(3, ({ int x[3]; *x=3; *(x+1)=4; *(2+x)=5; *x; }), "{ int x[3]; *x=3; *(x+1)=4; *(2+x)=5; *x; }");
    assert(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+1); }), "{ int x[3]; *x=3; *(x+1)=4; *(x+1); }");
    assert(5, ({ int x[3]; *x=3; *(x+1)=4; *(2+x)=5; *(x+2); }), "{ int x[3]; *x=3; *(x+1)=4; *(2+x)=5; *(x+2); }");
    assert(1, ({ int a[1]; *a=1; int *p=&a; *p; }), "{ int a[1]; *a=1; int *p=&a; *p; }");
    assert(3, ({ int a[2]; *a=1; *(a+1)=2; *a+*(a+1); }), "{ int a[2]; *a=1; *(a+1)=2; *a+*(a+1); }");
    assert(3, ({ int a[2]; *a=1; *(a+1)=2; int *p=&a; *p+*(p+1); }), "{ int a[2]; *a=1; *(a+1)=2; int *p=&a; *p+*(p+1); }");

    assert(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }), "{ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }");
    assert(1, ({ int x[2][3]; x[0][1]=1; x[0][1]; }), "{ int x[2][3]; x[0][1]=1; x[0][1]; }");
    assert(2, ({ int x[2][3]; x[0][2]=2; x[0][2]; }), "{ int x[2][3]; x[0][2]=2; x[0][2]; }");
    assert(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }), "{ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }");
    assert(4, ({ int x[2][3]; x[1][1]=4; x[1][1]; }), "{ int x[2][3]; x[1][1]=4; x[1][1]; }");
    assert(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }), "{ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }");
    assert(6, ({ int x[2][3]; int *y=x; y[6]=6; x[2][0]; }), "{ int x[2][3]; int *y=x; y[6]=6; x[2][0]; }");

    assert(1, ({ g1=1; g1; }), "{ g1=1; g1; }");
    assert(2, ({ g2[0]=2; g2[1]=3; g2[2]=4; g2[0]; }), "{ g2[0]=2; g2[1]=3; g2[2]=4; g2[0]; }");
    assert(3, ({ g2[0]=2; g2[1]=3; g2[2]=4; g2[1]; }), "{ g2[0]=2; g2[1]=3; g2[2]=4; g2[1]; }");
    assert(4, ({ g2[0]=2; g2[1]=3; g2[2]=4; g2[2]; }), "{ g2[0]=2; g2[1]=3; g2[2]=4; g2[2]; }");

    assert(4, ({ sizeof(g1); }), "{ sizeof(g1); }");
    assert(8, ({ sizeof(g3); }), "{ sizeof(g3); }");

    assert(1, ({ char x; sizeof(x); }), "{ char x; sizeof(x); }");
    assert(10, ({ char x[10]; sizeof(x); }), "{ char x[10]; sizeof(x); }");

    assert(97, ({ char *x="abc"; x[0]; }), "{ char *x=\"abc\"; x[0]; }"); // ascii
    assert(98, ({ "abc"[1]; }), "{ \"abc\"[1]; }");
    assert(99, ({ char *x="abc"; x[2]; }), "{ char *x=\"abc\"; x[2]; }");
    assert(0, ({ char *x="abc"; x[3]; }), "{ char *x=\"abc\"; x[3]; }");
    assert(4, ({ sizeof("abc"); }), "{ sizeof(\"abc\"); }");

    assert(7, ({ "\a"[0]; }), "{ \"\\a\"[0]; }");
    assert(8, ({ "\b"[0]; }), "{ \"\\b\"[0]; }");
    assert(12, ({ "\f"[0]; }), "{ \"\\f\"[0]; }");
    assert(13, ({ "\r"[0]; }), "{ \"\\r\"[0]; }");
    assert(9, ({ "\t"[0]; }), "{ \"\\t\"[0]; }");
    assert(11, ({ "\v"[0]; }), "{ \"\\v\"[0]; }");
    assert(10, ({ "\n"[0]; }), "{ \"\\n\"[0]; }");

    assert(3, ({ int x=2; x++; }), "{ int x=2; x++; }");
    assert(1, ({ int x=2; x--; }), "{ int x=2; x--; }");
    // assert(2, ({ int x=2; int y=x++; y; }), "{ int x=2; int y=x++; y; }");
    // assert(2, ({ int x=2; int y=x--; y; }), "{ int x=2; int y=x--; y; }");

    assert(5, ({ int x=2; x+=3; x; }), "{ int x=2; x+=3; x; }");
    assert(2, ({ int x=3; x-=1; x; }), "{ int x=3; x-=1; x; }");
    assert(8, ({ int x=4; x*=2; x; }), "{ int x=4; x*=2; x; }");
    assert(3, ({ int x=6; x/=2; x; }), "{ int x=6; x/=2; x; }");

    assert(8, ({ int x=6, y=2; x+y; }), "{ int x=6, y=2; x+y; }");
    assert(7, ({ int x=4, y[2]; y[0]=1; y[1]=3; x+y[1]; }), "{ int x=4, y[2]; y[0]=1; y[1]=3; x+y[1]; }");

    assert(24, ({ int *x[3]; sizeof(x); }), "{ int *x[3]; sizeof(x); }");
    assert(8, ({ int (*x)[3]; sizeof(x); }), "{ int (*x)[3]; sizeof(x); }");

    assert(3, ({ int i=2, j=3; i=j,6; i;}), "{ int i=2, j=3; i=j,6; i;}");
    assert(6, ({ int i=2, j=3; i=(j,6); i;}), "{ int i=2, j=3; i=(j,6); i;}");
    assert(6, ({ int i=2, j=3; i=(4,5,6); i;}), "{ int i=2, j=3; i=(4,5,6); i;}");

    printf("OK\n");
    return 0;
}
