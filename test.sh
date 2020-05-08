#!/bin/bash
cat <<EOF | cc -x c -c -o /tmp/tmpfs/tmp2.o -
int testFunc1() { return 5; }
int testFunc2(int x, int y) { return x+y; }
int testFunc3(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f;}

void alloc4(int **p, int a, int b, int c, int d) { int dummy[4]={a,b,c,d}; *p=dummy; }
EOF

assert() {
    expected="$1"
    input="$2"

    echo "$input" | ./mmcc - > tmp.s
    cc -static -o /tmp/tmpfs/tmp tmp.s /tmp/tmpfs/tmp2.o
    cp /tmp/tmpfs/tmp tmp
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 'int main() { return 0; }'
assert 42 'int main() { return 42; }'

assert 4 'int main() { return 5+1-2; }'
assert 15 'int main() { return 5*(9-6); }'
assert 4 'int main() { return (3+5)/2; }'

assert 10 'int main() { return -10+20; }'
assert 15 'int main() { return -3*-5; }'
assert 20 'int main() { return - - +20; }'

assert 1 'int main() { return 3==3; }'
assert 0 'int main() { return 4!=4; }'
assert 1 'int main() { return 5>3; }'
assert 0 'int main() { return 6>=9; }'
assert 1 'int main() { return 4<5; }'
assert 0 'int main() { return 6<=3; }'

assert 5 'int main() { int a; a=5; return a; }'
assert 1 'int main() { int c=5>3; return c; }'
assert 22 'int main() { int a; int b; a=13; b=a+9; return b; }'

assert 10 'int main() { if (5>3) { return 10; } }'
assert 20 'int main() { if (5<3) { return 10; } else { return 20; } }'
assert 30 'int main() { int foo=5; if (5<3) { return 10; } else { if (foo==4) { return 20; } else { return 30; } } }'
assert 40 'int main() { int foo=5; if (5>3) { if (foo==5) { return 40; } else { return 20; } } else { return 30; } }'

assert 5 'int main() { int i=1; while (i<5) { i=i+1; } { return i; } }'
assert 10 'int main() { int x=0; for (int i=0; i<10; i=i+1) { x=x+1; } { return x; } }'
assert 55 'int main() { int j=0; for (int i=0; i<=10; i=i+1) { j=i+j; } { return j; } }'
assert 3 'int main() { for (;;) { return 3; } { return 5; } }'
assert 100 'int main() { int a=0; for (int i=0; i<10; i=i+1) { if (i==5) { a=100; return a; } } }'

assert 6 'int main() { int a; int b; int c; if (5>3) { a=3; b=2; c=a*b; } { return c; } }'
assert 100 'int main() { int ret=0; for (int i=0; i<10; i=i+1) { int j=0; while (j<10) { ret=ret+1; j=j+1; } } return ret; }'

assert 5 'int main() { return testFunc1(); }'
assert 3 'int main() { return testFunc2(1, 2); }'
assert 21 'int main() { return testFunc3(1, 2, 3, 4, 5, 6); }'

assert 3 'int main() { return ret(); } int ret() { return 3; }'
assert 5 'int main() { return ret(); } int ret() { int a=5; return a; }'

assert 3 'int main() { return ret(1, 2); } int ret(int x, int y) { return x+y; }'
assert 21 'int main() { return ret(1, 2, 3, 4, 5, 6); } int ret(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }'
assert 8 'int main() { return fib(6); } int fib(int n) { if (n<=2) { return 1; } else { return fib(n-1)+fib(n-2); } }'

assert 3 'int main() { int x=3; return *&x; }'
assert 3 'int main() { int x=3; int *y=&x; int **z=&y; return **z; }'
assert 3 'int main() { int x; int *y=&x; *y=3; return x; }'

assert 5 'int main() { int x=3; int y=5; return *(&x+1); }'
assert 3 'int main() { int x=3; int y=5; int *z=&y-1; return *z; }'
assert 3 'int main() { int x=3; int y=5; return *(&y-1); }'
assert 7 'int main() { int x=3; int y=5; *(&x+1)=7; return y; }'
assert 7 'int main() { int x=3; int y=5; *(&y-1)=7; return x; }'

assert 4 'int main() { int *p; alloc4(&p, 1, 2, 4, 8); int *q=p+2; return *q; }'
assert 8 'int main() { int *p; alloc4(&p, 1, 2, 4, 8); int *q=3+p; return *q; }'
assert 1 'int main() { int *p; alloc4(&p, 1, 2, 4, 8); int *q=p+2; return *(q-2); }'
assert 2 'int main() { int *p; alloc4(&p, 1, 2, 4, 8); int *q=3+p; return *(q-2); }'

assert 4 'int main() { int x; return sizeof(x); }'
assert 8 'int main() { int *y; return sizeof(y); }'
assert 4 'int main() { return sizeof(1); }'
assert 4 'int main() { return sizeof(sizeof(1)); }'
assert 4 'int main() { int x; return sizeof(x+3); }'
assert 8 'int main() { int *y; return sizeof(y+3); }'
assert 4 'int main() { int *y; return sizeof(*y); }'
assert 20 'int main() { int x[5]; return sizeof(x); }'
assert 16 'int main() { int x[2][2]; return sizeof(x); }'

assert 3 'int main() { int x[2]; int *y=&x; *y=3; return *x; }'
assert 3 'int main() { int x[3]; *x=3; *(x+1)=4; *(2+x)=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; *(x+1)=4; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; *(x+1)=4; *(2+x)=5; return *(x+2); }'
assert 1 'int main() { int a[1]; *a=1; int *p=&a; return *p; }'
assert 3 'int main() { int a[2]; *a=1; *(a+1)=2; return *a+*(a+1); }'
assert 3 'int main() { int a[2]; *a=1; *(a+1)=2; int *p=&a; return *p+*(p+1); }'

assert 0 'int main() { int x[2][3]; int *y=x; y[0]=0; return x[0][0]; }'
assert 1 'int main() { int x[2][3]; x[0][1]=1; return x[0][1]; }'
assert 2 'int main() { int x[2][3]; x[0][2]=2; return x[0][2]; }'
assert 3 'int main() { int x[2][3]; int *y=x; y[3]=3; return x[1][0]; }'
assert 4 'int main() { int x[2][3]; x[1][1]=4; return x[1][1]; }'
assert 5 'int main() { int x[2][3]; int *y=x; y[5]=5; return x[1][2]; }'
assert 6 'int main() { int x[2][3]; int *y=x; y[6]=6; return x[2][0]; }'

assert 1 'int a; int main() { a=1; return a; }'
assert 2 'int a[3]; int main() { a[0]=2; a[1]=3; a[2]=4; return a[0]; }'
assert 3 'int a[3]; int main() { a[0]=2; a[1]=3; a[2]=4; return a[1]; }'
assert 4 'int a[3]; int main() { a[0]=2; a[1]=3; a[2]=4; return a[2]; }'
assert 5 'int x; int y; int main() { x=2; y=3; return x+y; }'
assert 6 'int x; int main() { x=2; return ret(); } int ret() { return x*3; }'

assert 4 'int x; int main() { return sizeof(x); }'
assert 8 'int *y; int main() { return sizeof(y); }'

assert 3 'int main() { char x[3]; x[0]=-1; x[1]=2; int y=4; return x[0]+y; }'
assert 2 'int main() { char x=1; char y=2; return y; }'
assert 1 'int main() { char x; return sizeof(x); }'
assert 10 'int main() { char x[10]; return sizeof(x); }'
assert 2 'int main() { return sub_char(7,1,1,1,1,1); } int sub_char(char a, char b, char c, char d, char e, char f) { return a-b-c-d-e-f; }'

assert 97 'int main() { char *x="abc"; return x[0]; }' # ascii
assert 98 'int main() { return "abc"[1]; }'
assert 99 'int main() { char *x="abc"; return x[2]; }'
assert 0 'int main() { char *x="abc"; return x[3]; }'
assert 4 'int main() { return sizeof("abc"); }'

assert 1 'int main() { int a=1; /* a=2; */ return a; }'

assert 1 'int main() { return ({ 0; 1; }); }'
assert 1 'int main() { ({ 0; return 1; 2; }); }'
assert 1 'int main() { ({ 0; return 1; 2; }); return 3; }'
assert 1 'int main() { return ({ int x=1; x; }); }'

echo OK
