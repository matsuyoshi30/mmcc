#!/bin/bash -x
set -e

TMP=tmp-self

rm -rf $TMP
mkdir -p $TMP

mmcc() {
	  file=$1
    cat <<EOF > $TMP/$1
typedef struct FILE FILE;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int *__errno_location();

typedef struct {
  int gp_offset;
  int fp_offset;
  void *overflow_arg_area;
  void *reg_save_area;
} __va_elem;
typedef __va_elem va_list[1];
int vfprintf(FILE *stderr, char *fmt, va_list ap);
int vprintf(char *fmt, va_list ap);
static void va_end(__va_elem *ap) {}
EOF

	  grep -v '^#' mmcc.h >> $TMP/$1
	  grep -v '^#' $1 >> $TMP/$1
    sed -i 's/\bbool\b/_Bool/g' $TMP/$1
    sed -i 's/\btrue\b/1/g; s/\bfalse\b/0/g;' $TMP/$1
    sed -i 's/\bNULL\b/0/g' $TMP/$1
    sed -i 's/\berrno\b/*__errno_location()/g' $TMP/$1
    sed -i 's/\bva_start\b/__builtin_va_start/g' $TMP/$1

	  ./mmcc $TMP/$1 > $TMP/${1%.c}.s
    gcc -g -c -o $TMP/${1%.c}.o $TMP/${1%.c}.s
}

cc() {
    gcc -c -o $TMP/${1%.c}.o $1
}

mmcc main.c
mmcc tokenize.c
mmcc type.c
mmcc parse.c
mmcc codegen.c

gcc -g -static -o mmcc-gen2 $TMP/*.o
