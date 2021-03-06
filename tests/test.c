int assert(long expected, long actual, char *code) {
    if (expected == actual) {
        printf("%s => %ld\n", code, actual);
    } else {
        printf("%s => %ld expected, but got %ld\n", code, expected, actual);
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

void testFunc4(int a) {
    if (a != 5)
        printf("testFunc4\n");
    return;
}

int g1;
int g2[4];
int *g3;

int f();

typedef int GInt;
typedef struct { int a; int b; } GStruct;

typedef enum {
    ZERO,
    ONE,
    TWO,
} GEnum;

long testFunc5(long a, long b) {
    return a + b;
}

short testFunc6(short a, short b) {
    return a - b;
}

static int static_func() {
    return 3;
}

extern int ext1;
extern int *ext2;

int;
struct {char a; int b;};
typedef struct { char a; int b; } Ty1;

char initExt1 = 5;
short initExt2 = 6;
int initExt3 = 7;
long initExt4 = 8;
int *initExt5 = &initExt3;
char *initExt6 = "abc";

int initExt7[3] = {0, 1, 2};
char *initExt8[] = {"foo", "bar"};

static int initExt9 = 3;

int initExt10[3] = {0, 1, 2};
char *initExt11[] = {"foo", "bar"};
struct {char a; int b;} initExt12[2] = {{1, 2}, {3, 4}};
struct {int a[2];} initExt13[2] = {{{1, 2}}};
struct {int a[2];} initExt14[2] = {{1, 2}, 3, 4};
struct {int a[2];} initExt15[2] = {1, 2, 3, 4};

int counter() {
  static int i;
  static int j = 1+1;
  return i++ + j++;
}

int add_all1(int x, ...);
int add_all3(int z, int b, int c, ...);

typedef struct {
  int gp_offset;
  int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} va_list[1];

int sprintf(char *buf, char *fmt, ...);
int vsprintf(char *buf, char *fmt, va_list ap);

char *fmt(char *buf, char *fmt, ...) {
  va_list ap;
  __builtin_va_start(ap, fmt);
  vsprintf(buf, fmt, ap);
}

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

    assert(8, ({ int x=6, y=2; x+y; }), "{ int x=6, y=2; x+y; }");
    assert(7, ({ int x=4, y[2]; y[0]=1; y[1]=3; x+y[1]; }), "{ int x=4, y[2]; y[0]=1; y[1]=3; x+y[1]; }");

    assert(24, ({ int *x[3]; sizeof(x); }), "{ int *x[3]; sizeof(x); }");
    assert(8, ({ int (*x)[3]; sizeof(x); }), "{ int (*x)[3]; sizeof(x); }");

    assert(3, ({ int *x[3]; int y; x[0]=&y; y=3; x[0][0]; }), "{ int *x[3]; int y; x[0]=&y; y=3; x[0][0]; }");
    assert(4, ({ int x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; }), "{ int x[3]; int (*y)[3]=x; y[0][0]=4; y[0][0]; }");

    assert(3, ({ int i=2, j=3; i=j,6; i;}), "{ int i=2, j=3; i=j,6; i;}");
    assert(6, ({ int i=2, j=3; i=(j,6); i;}), "{ int i=2, j=3; i=(j,6); i;}");
    assert(6, ({ int i=2, j=3; i=(4,5,6); i;}), "{ int i=2, j=3; i=(4,5,6); i;}");

    assert(1, ({ struct { int n; } a; a.n=1; a.n; }), "{ struct { int n; } a; a.n=1; a.n; }");
    assert(4, ({ struct { int n; } a; sizeof(a); }), "{ struct { int n; } a; sizeof(a); }");
    assert(3, ({ struct { int m; int n; } a; a.m=1; a.n=2; a.m+a.n; }), "{ struct { int m; int n; } a; a.m=1; a.n=2; a.m+a.n; }");
    assert(8, ({ struct { int m; int n; } a; sizeof(a); }), "{ struct { int m; int n; } a; sizeof(a); }");

    assert(0, ({ struct { int a; int b; } x[3]; int *p=x; p[0]=0; x[0].a; }), "{ struct { int a; int b; } x[3]; int *p=x; p[0]=0; x[0].a; }");
    assert(1, ({ struct { int a; int b; } x[3]; int *p=x; p[1]=1; x[0].b; }), "{ struct { int a; int b; } x[3]; int *p=x; p[1]=1; x[0].b; }");
    assert(2, ({ struct { int a; int b; } x[3]; int *p=x; p[2]=2; x[1].a; }), "{ struct { int a; int b; } x[3]; int *p=x; p[2]=2; x[1].a; }");
    assert(3, ({ struct { int a; int b; } x[3]; int *p=x; p[3]=3; x[1].b; }), "{ struct { int a; int b; } x[3]; int *p=x; p[3]=3; x[1].b; }");

    assert(6, ({ struct { int a[3]; int b[5]; } x; int *p=&x; x.a[0]=6; p[0]; }), "{ struct { int a[3]; int b[5]; } x; int *p=&x; x.a[0]=6; p[0]; }");
    assert(7, ({ struct { int a[3]; int b[5]; } x; int *p=&x; x.a[1]=7; p[1]; }), "{ struct { int a[3]; int b[5]; } x; int *p=&x; x.a[1]=7; p[7]; }");
    assert(8, ({ struct { int a[3]; int b[5]; } x; int *p=&x; x.b[0]=8; p[3]; }), "{ struct { int a[3]; int b[5]; } x; int *p=&x; x.b[0]=8; p[3]; }");
    assert(9, ({ struct { int a[3]; int b[5]; } x; int *p=&x; x.b[1]=9; p[4]; }), "{ struct { int a[3]; int b[5]; } x; int *p=&x; x.b[1]=9; p[4]; }");

    assert(1, ({ struct { struct { int n; } b; } a; a.b.n=1; a.b.n; }), "{ struct { struct { int n; } b; } a; a.b.n=1; a.b.n; }");
    assert(4, ({ struct { struct { int n; } b; } a; sizeof(a); }), "{ struct { struct { int n; } b; } a; sizeof(a); }");
    assert(3, ({ struct { int m; struct { int n; } b; } a; a.m=1; a.b.n=2; a.m+a.b.n; }), "{ struct { int m; struct { int n; } b; } a; a.m=1; a.b.n=2; a.m+a.b.n; }");
    assert(8, ({ struct { int m; struct { int n; } b; } a; sizeof(a); }), "{ struct { int m; struct { int n; } b; } a; sizeof(a); }");

    assert(8, ({ struct t { int a; int b; } x; struct t y; sizeof(y); }), "{ struct t { int a; int b; } x; struct t y; sizeof(y); }");
    assert(8, ({ struct t { int a; int b; }; struct t y; sizeof(y); }), "{ struct t { int a; int b; }; struct t y; sizeof(y); }");
    assert(32, ({ struct t { int a[3]; int b[5]; } x; struct t y; sizeof(y); }), "{ struct { int a[3]; int b[5]; } x; struct t y; sizeof(y); }");
    assert(6, ({ struct t { int a[3]; int b[5]; } x; struct t y; int *p=&y; y.a[0]=6; p[0]; }), "{ struct { int a[3]; int b[5]; } x; struct t y; int *p=&y; y.a[0]=6; p[0]; }");

    assert(2, ({ int x=2; { int x=3; } x; }), "{ int x=2; { int x=3; } x; }");
    assert(3, ({ int x=2; { x=3; } x; }), "{ int x=2; { x=3; } x; }");
    assert(4, ({ struct t { int a; } x; { struct t { char c; } y; } sizeof(x); }), "{ struct t { int a; } x; struct t { char c; } y; } sizeof(x); }");
    assert(3, ({ struct t { int a; } x; int t=1; struct t y; y.a=2; t+y.a; }), "{ struct t { int a; } x; int t=1; struct t y; y.a=2; t+y.a; }");

    assert(1, ({ struct t { int n; } a; struct t *b=&a; b->n=1; b->n; }), "{ struct t { int n; } a; struct t *b=&a; b->n=1; b->n; }");
    assert(1, ({ struct t { int n; } a; struct t *b=&a; b->n=1; a.n; }), "{ struct t { int n; } a; struct t *b=&a; b->n=1; a.n; }");
    assert(1, ({ struct t { int n; } a; struct t *b=&a; a.n=1; b->n; }), "{ struct t { int n; } a; struct t *b=&a; a.n=1; b->n; }");

    assert(1, ({ typedef int MyInt; MyInt x=1; x; }), "{ typedef MyInt int; MyInt x=1; x; }");
    assert(1, ({ typedef struct { int n; } MyStruct; MyStruct x; x.n=1; x.n; }), "{ typedef struct { int n; } MyStruct; MyStruct x; x.n=1; x.n; }");
    assert(2, ({ GInt x=2; x; }), "{ GInt x=2; x; }");
    assert(3, ({ GStruct x; x.a=1; x.b=2; x.a+x.b; }), "{ GStruct x; x.a=1; x.b=2; x.a+x.b; }");
    assert(3, ({ typedef int t; t t=3; t; }), "{ typedef int t; t t=3; t; }");
    assert(5, ({ typedef int a, b; a x=3; b y=2; x+y; }), "{ typedef int a, b; a x=3; b y=2; x+y; }");

    printf("void test passed\n", testFunc4(5));

    assert(97, 'a', "'a'");
    assert(10, '\n', "'\\n'");
    assert(0, '\0', "'\\0'");
    assert(4, ({ sizeof('a'); }), "{ sizeof('a'); }");

    assert(0, ({ enum { zero, one, two }; zero; }), "{ enum { zero, one, two }; zero; }");
    assert(1, ({ enum { zero, one, two }; one; }), "{ enum { zero, one, two }; one; }");
    assert(2, ({ enum { zero, one, two }; two; }), "{ enum { zero, one, two }; two; }");
    assert(5, ({ enum { zero, one, two, five=5, six, seven }; five; }), "{ enum { zero, one, two, five=5, six, seven }; five; }");
    assert(6, ({ enum { zero, one, two, five=5, six, seven }; six; }), "{ enum { zero, one, two, five=5, six, seven }; six; }");
    assert(7, ({ enum { zero, one, two, five=5, six, seven }; seven; }), "{ enum { zero, one, two, five=5, six, seven }; seven; }");
    assert(4, ({ enum { zero, one, two } x; sizeof(x); }), "({ enum { zero, one, two } x; sizeof(x); })");
    assert(4, ({ enum t { zero, one, two }; enum t x; sizeof(x); }), "({ enum t { zero, one, two }; enum t x; sizeof(x); })");

    assert(0, ({ GEnum ge = ZERO; ge; }), "{ GEnum ge = ZERO; ge; }");
    assert(1, ({ GEnum ge; ge = ONE; ge; }), "{ GEnum ge; ge = ONE; ge; }");

    assert(0, !1, "!1");
    assert(0, !2, "!2");
    assert(1, !0, "!0");

    assert(2, ({ int x=2; x++; }), "{ int x=2; x++; }");
    assert(2, ({ int x=2; x--; }), "{ int x=2; x--; }");
    assert(2, ({ int x=2; int y=x++; y; }), "{ int x=2; int y=x++; y; }");
    assert(2, ({ int x=2; int y=x--; y; }), "{ int x=2; int y=x--; y; }");
    assert(3, ({ int x=2; x++; x; }), "{ int x=2; x++; x; }");
    assert(1, ({ int x=2; x--; x; }), "{ int x=2; x--; x; }");

    assert(3, ({ int x=2; ++x; }), "{ int x=2; ++x; }");
    assert(1, ({ int x=2; --x; }), "{ int x=2; --x; }");

    assert(5, ({ int x=2; x+=3; x; }), "{ int x=2; x+=3; x; }");
    assert(5, ({ int x=2; x+=3; }), "{ int x=2; x+=3; }");
    assert(2, ({ int x=3; x-=1; x; }), "{ int x=3; x-=1; x; }");
    assert(2, ({ int x=3; x-=1; }), "{ int x=3; x-=1; }");
    assert(8, ({ int x=4; x*=2; x; }), "{ int x=4; x*=2; x; }");
    assert(8, ({ int x=4; x*=2; }), "{ int x=4; x*=2; }");
    assert(3, ({ int x=6; x/=2; x; }), "{ int x=6; x/=2; x; }");
    assert(3, ({ int x=6; x/=2; }), "{ int x=6; x/=2; }");

    assert(1, 0||1, "0||1");
    assert(0, 0||0, "0||0");
    assert(0, 0&&1, "0&&1");
    assert(1, 1&&5, "1&&5");

    assert(6, ({ int x=0; for (int i=0; i<10; i=i+1) { x=x+1; if (i==5) break; } x; }), "{ int x=0; for (int i=0; i<10; i=i+1) { x=x+1; if (i==5) break; } x; }");
    assert(5, ({ int i=0; while (1) { i++; if (i==5) break; } i; }), "{ int i=0; while (1) { i++; if (i==5) break; } i; }");

    assert(6, ({ int x=0; for (int i=0;i<10;i++) { if (i>5) continue; x++; } x; }), "{ int x=0; for (int i=0;i<10;i++) { if (i>5) continue; x++; } x; }");
    assert(5, ({ int x=0; int i=0; while (i<10) { i++; if (i>5) continue; x++; } x; }), "{ int x=0; int i=0; while (i<10) { i++; if (i>5) continue; x++; } x; }");


    assert(3, ({ int i=0; goto a; a: i++; b: i++; c: i++; i; }), "{ int i=0; goto a; a: i++; b: i++; c: i++; i; }");
    assert(2, ({ int i=0; goto e; d: i++; e: i++; f: i++; i; }), "{ int i=0; goto e; d: i++; e: i++; f: i++; i; }");
    assert(1, ({ int i=0; goto i; g: i++; h: i++; i: i++; i; }), "{ int i=0; goto i; g: i++; h: i++; i: i++; i; }");

    assert(5, ({ int i=0; switch(0) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }), "{ int i=0; switch(0) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }");
    assert(6, ({ int i=0; switch(1) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }), "{ int i=0; switch(1) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }");
    assert(7, ({ int i=0; switch(2) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }), "{ int i=0; switch(2) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }");
    assert(8, ({ int i=0; switch(3) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }), "{ int i=0; switch(3) { case 0: i=5; break; case 1: i=6; break; case 2: i=7; break; default: i=8; } i; }");

    assert(97, (int)'a', "(int)'a'");
    assert(131585, (int)8590066177, "(int)8590066177");
    assert(513, (short)8590066177, "(short)8590066177");
    assert(1, (char)8590066177, "(char)8590066177");
    assert(1, (long)1, "(long)1");

    assert(2, ({ short x=2; x; }), "{ short x=2; x; }");
    assert(2, ({ short x; sizeof(x); }), "{ short x; sizeof(x); }");
    assert(8, ({ long x=8; x; }), "{ long x=8; x; }");
    assert(8, ({ long x; sizeof(x); }), "{ long x; sizeof(x); }");

    assert(3, (int)testFunc5((long)1, (long)2), "(int)testFunc5((long)1, (long)2)");
    assert(4, (int)testFunc6((short)7, (short)3), "(int)testFunc6((short)7, (short)3)");

    assert(0, ({ _Bool x=0; x; }), "{ _Bool x=0; x; }");
    assert(1, ({ _Bool x=1; x; }), "{ _Bool x=1; x; }");
    assert(1, ({ _Bool x=2; x; }), "{ _Bool x=2; x; }");
    assert(1, (_Bool)1, "(_Bool)1");
    assert(1, (_Bool)2, "(_Bool)2");
    assert(0, (_Bool)(char)256, "(_Bool)(char)256");

    assert(1, sizeof(char), "sizeof(char)");
    assert(2, sizeof(short), "sizeof(short)");
    assert(4, sizeof(int), "sizeof(int)");
    assert(8, sizeof(long), "sizeof(long)");
    assert(8, sizeof(char *), "sizeof(char *)");
    assert(8, sizeof(int *), "sizeof(int *)");
    assert(8, sizeof(long *), "sizeof(long *)");
    assert(8, sizeof(int **), "sizeof(int **)");
    assert(8, sizeof(int(*)[4]), "sizeof(int(*)[4])");
    assert(32, sizeof(int*[4]), "sizeof(int*[4])");
    assert(16, sizeof(int[4]), "sizeof(int[4])");
    assert(48, sizeof(int[3][4]), "sizeof(int[3][4])");
    assert(8, sizeof(struct {int a; int b;}), "sizeof(struct {int a; int b;})");

    assert(4, ({ struct T *foo; struct T {int x;}; sizeof(struct T); }), "{ struct T *foo; struct T {int x;}; sizeof(struct T); }");
    assert(4, ({ typedef struct T T; struct T { int x; }; sizeof(T); }), "{ typedef struct T T; struct T { int x; }; sizeof(T); }");

    assert(3, static_func(), "static_func()");

    ext1 = 5;
    assert(5, ext1, "ext1");
    ext2 = &ext1;
    assert(5, *ext2, "*ext2");

    assert(0, 12%2, "12%2");
    assert(5, 17%6, "17%6");
    assert(10, 5+(26%7), "5+(26%7)");
    assert(2, ({ int i=10; i%=4; i; }), "{ int i=10; i%=4; i; }");

    assert(8, sizeof(int(*)[]), "sizeof(int(*)[])");
    assert(8, sizeof(int(*)[][10]), "sizeof(int(*)[][10])");

    assert(1, ({ int x[]={1,2,3,4}; x[0]; }), "{ int x[]={1,2,3,4}; x[0]; }");
    assert(2, ({ int x[]={1,2,3,4}; x[1]; }), "{ int x[]={1,2,3,4}; x[1]; }");
    assert(3, ({ int x[]={1,2,3,4}; x[2]; }), "{ int x[]={1,2,3,4}; x[2]; }");
    assert(4, ({ int x[]={1,2,3,4}; x[3]; }), "{ int x[]={1,2,3,4}; x[3]; }");
    assert(4, ({ char x[]="foo"; sizeof(x); }), "{ char x[]=\"foo\"; sizeof(x); }");
    assert(24, ({ char *x[]={"foo", "bar", "hoge"}; sizeof(x); }), "{ char *x[]={\"foo\", \"bar\", \"hoge\"}; sizeof(x); }");

    assert(2, 0?1:2, "0?1:2");
    assert(1, 1?1:2, "1?1:2");
    assert(-1, 0?-2:-1, "0?-2:-1");
    assert(-2, 1?-2:-1, "1?-2:-1");

    assert(1, ({ int x[3]={1,2,3}; x[0]; }), "{ int x[3]={1,2,3}; x[0]; }");
    assert(2, ({ int x[3]={1,2,3}; x[1]; }), "{ int x[3]={1,2,3}; x[0]; }");
    assert(2, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[0][1]; }), "{ int x[2][3]={{1,2,3},{4,5,6}}; x[0][1]; }");
    assert(4, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][0]; }), "{ int x[2][3]={{1,2,3},{4,5,6}}; x[1][0]; }");
    assert(6, ({ int x[2][3]={{1,2,3},{4,5,6}}; x[1][2]; }), "{ int x[2][3]={{1,2,3},{4,5,6}}; x[1][2]; }");

    assert('a', ({ char x[4]="abc"; x[0]; }), "{ char x[4]=\"abc\"; x[0]; }");
    assert('c', ({ char x[4]="abc"; x[2]; }), "{ char x[4]=\"abc\"; x[2]; }");
    assert(0, ({ char x[4]="abc"; x[3]; }), "{ char x[4]=\"abc\"; x[3]; }");
    assert('a', ({ char x[2][4]={"abc","def"}; x[0][0]; }), "{ char x[2][4]=\"abc\",\"def\"}; x[0][0]; }");
    assert(0, ({ char x[2][4]={"abc","def"}; x[0][3]; }), "{ char x[2][4]=\"abc\",\"def\"}; x[0][3]; }");
    assert('d', ({ char x[2][4]={"abc","def"}; x[1][0]; }), "{ char x[2][4]=\"abc\",\"def\"}; x[1][0]; }");
    assert('f', ({ char x[2][4]={"abc","def"}; x[1][2]; }), "{ char x[2][4]=\"abc\",\"def\"}; x[1][2]; }");

    assert(1, ({ struct {int a; int b; int c;} x={1,2,3}; x.a; }), "struct {int a; int b; int c;} x={1,2,3}; x.a;");
    assert(2, ({ struct {int a; int b; int c;} x={1,2,3}; x.b; }), "struct {int a; int b; int c;} x={1,2,3}; x.b;");
    assert(3, ({ struct {int a; int b; int c;} x={1,2,3}; x.c; }), "struct {int a; int b; int c;} x={1,2,3}; x.c;");

    assert(1, ({ struct {int a; int b; int c;} x={1}; x.a; }), "struct {int a; int b; int c;} x={1}; x.a;");
    assert(0, ({ struct {int a; int b; int c;} x={1}; x.b; }), "struct {int a; int b; int c;} x={1}; x.b;");
    assert(0, ({ struct {int a; int b; int c;} x={1}; x.c; }), "struct {int a; int b; int c;} x={1}; x.c;");

    assert(1, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].a; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].a;");
    assert(2, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].b; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[0].b;");
    assert(3, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].a; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].a;");
    assert(4, ({ struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].b; }), "struct {int a; int b;} x[2]={{1,2},{3,4}}; x[1].b;");

    assert(0, ({ struct {int a; int b;} x[2]={{1,2}}; x[1].b; }), "struct {int a; int b;} x[2]={{1,2}}; x[1].b;");

    assert(0, ({ struct {int a; int b;} x={}; x.a; }), "struct {int a; int b;} x={}; x.a;");
    assert(0, ({ struct {int a; int b;} x={}; x.b; }), "struct {int a; int b;} x={}; x.b;");

    assert(10, ({ enum { ten=1+2+3+4, }; ten; }), "{ enum { ten=1+2+3+4, }; ten; }");
    assert(8, ({ int x[1+1]; sizeof(x); }), "{ int x[1+1]; sizeof(x); }");
    assert(2, ({ char x[1?2:3]; sizeof(x); }), "{ char x[0?2:3]; sizeof(x); }");
    assert(3, ({ char x[0?2:3]; sizeof(x); }), "{ char x[1?2:3]; sizeof(x); }");

    assert(5, initExt1, "initExt1");
    assert(6, initExt2, "initExt2");
    assert(7, initExt3, "initExt3");
    assert(8, initExt4, "initExt4");
    assert(7, *initExt5, "*initExt5");
    assert(0, strcmp(initExt6, "abc"), "strcmp(initExt6, \"abc\")");

    assert(0, initExt7[0], "initExt7[0]");
    assert(1, initExt7[1], "initExt7[1]");
    assert(2, initExt7[2], "initExt7[2]");

    assert(0, strcmp(initExt8[0], "foo"), "strcmp(initExt8[0], \"foo\")");
    assert(0, strcmp(initExt8[1], "bar"), "strcmp(initExt8[1], \"bar\")");
    assert(0, initExt8[1][3], "initExt8[1][3]");
    assert(2, sizeof(initExt8) / sizeof(*initExt8), "sizeof(initExt8) / sizeof(*initExt8)");

    assert(8, ({ char *x[] = {"foo", "bar", "hoge"}; sizeof(x[0]); }), "{ char *x[] = {\"foo\", \"bar\", \"hoge\"}; sizeof(x[0]); }");
    assert(8, ({ char *x[] = {"foo", "bar", "hoge"}; sizeof(x[1]); }), "{ char *x[] = {\"foo\", \"bar\", \"hoge\"}; sizeof(x[1]); }");
    assert(8, ({ char *x[] = {"foo", "bar", "hoge"}; sizeof(x[2]); }), "{ char *x[] = {\"foo\", \"bar\", \"hoge\"}; sizeof(x[2]); }");

    assert(3, initExt9, "initExt9");

    assert(0, initExt10[0], "initExt10[0]");
    assert(1, initExt10[1], "initExt10[1]");
    assert(2, initExt10[2], "initExt10[2]");

    assert(0, strcmp(initExt11[0], "foo"), "strcmp(initExt11[0], \"foo\")");
    assert(0, strcmp(initExt11[1], "bar"), "strcmp(initExt11[1], \"bar\")");
    assert(0, initExt11[1][3], "initExt11[1][3]");
    assert(2, sizeof(initExt11) / sizeof(*initExt11), "sizeof(initExt11) / sizeof(*initExt11)");

    assert(1, initExt12[0].a, "initExt12[0].a");
    assert(2, initExt12[0].b, "initExt12[0].b");
    assert(3, initExt12[1].a, "initExt12[1].a");
    assert(4, initExt12[1].b, "initExt12[1].b");

    assert(1, initExt13[0].a[0], "initExt13[0].a[0]");
    assert(2, initExt13[0].a[1], "initExt13[0].a[1]");
    assert(0, initExt13[1].a[0], "initExt13[1].a[0]");
    assert(0, initExt13[1].a[1], "initExt13[1].a[1]");

    assert(1, initExt14[0].a[0], "initExt14[0].a[0]");
    assert(2, initExt14[0].a[1], "initExt14[0].a[1]");
    assert(3, initExt14[1].a[0], "initExt14[1].a[0]");
    assert(4, initExt14[1].a[1], "initExt14[1].a[1]");

    assert(1, initExt15[0].a[0], "initExt15[0].a[0]");
    assert(2, initExt15[0].a[1], "initExt15[0].a[1]");
    assert(3, initExt15[1].a[0], "initExt15[1].a[0]");
    assert(4, initExt15[1].a[1], "initExt15[1].a[1]");

    assert(2, counter(), "counter()");
    assert(4, counter(), "counter()");
    assert(6, counter(), "counter()");

    assert(8, ({ struct {char a; int b;} x; sizeof(x); }), "{ struct {char a; int b;} x; sizeof(x); }");
    assert(16, ({ struct {int a; long b;} x; sizeof(x); }), "{ struct {int a; long b;} x; sizeof(x); }");

    assert(1, (int){1}, "(int){1}");
    assert(2, ((int[]){0,1,2})[2], "(int[]){0,1,2}[2]");
    assert('a', ((struct {char a; int b;}){'a', 3}).a, "((struct {char a; int b;}){'a', 3}).a");
    assert(3, ({ int x=3; (int){x}; }), "int x=3; (int){x};");

    assert(7, ({ int i=0; int j=0; do { j++; } while (i++ < 6); j; }), "{ int i=0; int j=0; do { j++; } while (i++ < 6); j; }");
    assert(4, ({ int i=0; int j=0; int k=0; do { if (++j > 3) break; continue; k++; } while (1); j; }), "{ int i=0; int j=0; int k=0; do { if (j++ > 3) break; continue; k++; } while (1); j; }");

    assert(6, add_all1(1,2,3,0), "add_all1(1,2,3,0)");
    assert(5, add_all1(1,2,3,-1,0), "add_all1(1,2,3,-1,0)");

    assert(6, add_all3(1,2,3,0), "add_all3(1,2,3,0)");
    assert(5, add_all3(1,2,3,-1,0), "add_all3(1,2,3,-1,0)");

    assert(0, ({ char buf[100]; sprintf(buf, "%d %d %s", 1, 2, "foo"); strcmp("1 2 foo", buf); }), "{ char buf[100]; sprintf(buf, \"%d %d %s\", 1, 2, \"foo\"); strcmp(\"1 2 foo\", buf); }");
    assert(0, ({ char buf[100]; fmt(buf, "%d %d %s", 1, 2, "foo"); strcmp("1 2 foo", buf); }), "{ char buf[100]; fmt(buf, \"%d %d %s\", 1, 2, \"foo\"); strcmp(\"1 2 foo\", buf); }");

    printf("OK\n");
    return 0;
}
