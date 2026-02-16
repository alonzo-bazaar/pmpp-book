#pragma once
#include<stdio.h>     // printf, fprintf, stderr, &Co.
#include<sysexits.h>  // for exit failure code
#include<stdlib.h>    // for the exit() function

// printing
void print_vec(size_t elts, float* vec) {
    putchar('[');
    for(size_t i = 0; i<elts-1; ++i)
	printf("%4.2f, ", vec[i]);

    printf("%4.2f", vec[elts-1]);
    putchar(']');
}

void println_vec(size_t elts, float* vec) {
    print_vec(elts, vec);
    putchar('\n');
}

void print_mat(size_t nrows, size_t ncols, float* mat) {
    if(nrows==1) {
	putchar('['); print_vec(ncols, mat); putchar(']');
	return;
    }

    putchar('[');
    float*row = mat;
    for(size_t i=0; i<nrows-1; i++, row+=ncols) {
	print_vec(ncols, row);
	putchar(','); putchar('\n'); putchar(' ');
    }
    print_vec(ncols, mat);
    putchar(']');
}

void println_mat(size_t nrows, size_t ncols, float* mat) {
    print_mat(nrows, ncols, mat);
    putchar('\n');
}
