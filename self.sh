#!/bin/bash -x
set -e

TMP=tmp-self

mkdir -p $TMP

mmcc() {
	  file=$1
    cat <<EOF > $TMP/$1
EOF

	  grep -v '^#' mmcc.h >> $TMP/$1
	  grep -v '^#' $1 >> $TMP/$1
    sed -i 's/\bbool\b/_Bool/g' $TMP/$1
    sed -i 's/, \.\.\.//g' $TMP/$1

	  ./mmcc $TMP/$1 > $TMP/${1%.c}.s
    gcc -c -o $TMP/${1%.c}.o $TMP/${1%.c}.s
}

cc() {
    gcc -c -o $TMP/${1%.c}.o $1
}

mmcc main.c
cc tokenize.c
cc parse.c
mmcc codegen.c

gcc -static -o mmcc-gen2 $TMP/*.o
