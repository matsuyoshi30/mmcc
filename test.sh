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

assert 0 '0;'
assert 42 '42;'
assert 4 '5+1-2;'
assert 10 ' 12 - 2 ;'
assert 12 '10+8/4;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 15 '-3*-5;'
assert 20 '- - +20;'
assert 1 '3==3;'
assert 0 '4!=4;'
assert 1 '5>3;'
assert 0 '6>=9;'
assert 1 '4<5;'
assert 0 '6<=3;'
assert 5 'a=5;'
assert 2 'b=3-1;'
assert 1 'c=5>3;'

echo OK
