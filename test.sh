#!/bin/bash
assert() {
    expected="$1"
    input="$2"

    ./mmcc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 '{ return 0; }'
assert 42 '{ return 42; }'

assert 4 '{ return 5+1-2; }'
assert 15 '{ return 5*(9-6); }'
assert 4 '{ return (3+5)/2; }'

assert 10 '{ return -10+20; }'
assert 15 '{ return -3*-5; }'
assert 20 '{ return - - +20; }'

assert 1 '{ return 3==3; }'
assert 0 '{ return 4!=4; }'
assert 1 '{ return 5>3; }'
assert 0 '{ return 6>=9; }'
assert 1 '{ return 4<5; }'
assert 0 '{ return 6<=3; }'

assert 5 '{ a=5; return a; }'
assert 1 '{ c=5>3; return c; }'
assert 10 '{ foo=10; return foo; }'
assert 123 '{ bar=10; bar=123; return bar; }'
assert 22 '{ a=13; b=a+9; return b; }'

assert 10 'if (5>3) { return 10; }'
assert 20 'if (5<3) { return 10; } else { return 20; }'
assert 30 'foo=5; if (5<3) { return 10; } else { if (foo==4) { return 20; } else { return 30; } }'
assert 40 'foo=5; if (5>3) { if (foo==5) { return 40; } else { return 20; } } else { return 30; }'

assert 5 'i=1; while (i<5) { i=i+1; } { return i; }'
assert 10 'x=0; for (i=0; i<10; i=i+1) { x=x+1; } { return x; }'
assert 55 'i=0; j=0; for (i=0; i<=10; i=i+1) { j=i+j; } { return j; }'
assert 3 'for (;;) { return 3; } { return 5; }'
assert 100 'a=0; for (i=0; i<10; i=i+1) { if (i==5) { a=100; return a; } }'

assert 6 'if (5>3) { a=3; b=2; c=a*b; } { return c; }'
assert 100 'ret=0; for (i=0; i<10; i=i+1) { j=0; while (j<10) { ret=ret+1; j=j+1; } } return ret;'

echo OK
