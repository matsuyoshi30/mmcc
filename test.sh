#!/bin/bash
cat <<EOF | cc -x c -c -o tmp2.o -
int testFunc1() { return 5; }
int testFunc2(int x, int y) { return x+y; }
int testFunc3(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f;}
EOF

assert() {
    expected="$1"
    input="$2"

    ./mmcc "$input" > tmp.s
    cc -fPIC -o tmp tmp.s tmp2.o
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
assert 1 'int main() { int c; c=5>3; return c; }'
assert 10 'int main() { int foo; foo=10; return foo; }'
assert 123 'int main() { int bar; bar=10; bar=123; return bar; }'
assert 22 'int main() { int a; int b; a=13; b=a+9; return b; }'

assert 10 'int main() { if (5>3) { return 10; } }'
assert 20 'int main() { if (5<3) { return 10; } else { return 20; } }'
assert 30 'int main() { int foo; foo=5; if (5<3) { return 10; } else { if (foo==4) { return 20; } else { return 30; } } }'
assert 40 'int main() { int foo; foo=5; if (5>3) { if (foo==5) { return 40; } else { return 20; } } else { return 30; } }'

assert 5 'int main() { int i; i=1; while (i<5) { i=i+1; } { return i; } }'
assert 10 'int main() { int x; x=0; int i; for (i=0; i<10; i=i+1) { x=x+1; } { return x; } }'
assert 55 'int main() { int i; int j; i=0; j=0; for (i=0; i<=10; i=i+1) { j=i+j; } { return j; } }'
assert 3 'int main() { for (;;) { return 3; } { return 5; } }'
assert 100 'int main() { int a; a=0; int i; for (i=0; i<10; i=i+1) { if (i==5) { a=100; return a; } } }'

assert 6 'int main() { int a; int b; int c; if (5>3) { a=3; b=2; c=a*b; } { return c; } }'
assert 100 'int main() { int ret; ret=0; int i; int j; for (i=0; i<10; i=i+1) { j=0; while (j<10) { ret=ret+1; j=j+1; } } return ret; }'

assert 5 'int main() { return testFunc1(); }'
assert 3 'int main() { return testFunc2(1, 2); }'
assert 21 'int main() { return testFunc3(1, 2, 3, 4, 5, 6); }'

assert 3 'int main() { return ret(); } int ret() { return 3; }'
assert 5 'int main() { return ret(); } int ret() { int a; a=5; return a; }'

assert 3 'int main() { return ret(1, 2); } int ret(int x, int y) { return x+y; }'
assert 21 'int main() { return ret(1, 2, 3, 4, 5, 6); } int ret(int a, int b, int c, int d, int e, int f) { return a+b+c+d+e+f; }'
assert 8 'int main() { return fib(6); } int fib(int n) { if (n <= 2) { return 1; } else { return fib(n-1) + fib(n-2); } }'

assert 3 'int main() { int x=3; return *&x; }'
assert 3 'int main() { int x=3; int *y=&x; int **z=&y; return **z; }'
assert 3 'int main() { int x; int *y=&x; *y=3; return x; }'

assert 5 'int main() { int x=3; int y=5; return *(&x+8); }'
assert 3 'int main() { int x=3; int y=5; int *z=&y-8; return *z; }'
assert 3 'int main() { int x=3; int y=5; return *(&y-8); }'
assert 7 'int main() { int x=3; int y=5; *(&x+8)=7; return y; }'
assert 7 'int main() { int x=3; int y=5; *(&y-8)=7; return x; }'

echo OK
