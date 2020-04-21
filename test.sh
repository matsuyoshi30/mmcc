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

assert 0 0
assert 42 42
assert 6 3+3
assert 3 4-1
assert 4 5+1-2
assert 5 4-2+3
assert 10 ' 12 - 2 '

echo OK
