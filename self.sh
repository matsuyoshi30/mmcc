#!/bin/bash -x
set -e

TMP=tmp-self

mkdir -p $TMP

mmcc() {
	  file=$1
	  cat <<EOF > $TMP/$1
typedef struct FILE FILE;
extern FILE *stdout;
extern FILE *stderr;

void *malloc(long size);
void *calloc(long nmemb, long size);
int *__errno_location();
char *strerror(int errnum);
FILE *fopen(char *pathname, char *mode);
long fread(void *ptr, long size, long nmemb, FILE *stream);
int feof(FILE *stream);
static void assert() {}
int strcmp(char *s1, char *s2);
EOF

	  grep -v '^#' mmcc.h >> $TMP/$1
	  grep -v '^#' $1 >> $TMP/$1
    sed -i 's/\bbool\b/_Bool/g' $TMP/$1
    sed -i 's/\berrno\b/*__errno_location()/g' $TMP/$1
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
cc codegen.c

gcc -static -o mmcc-gen2 $TMP/*.o
