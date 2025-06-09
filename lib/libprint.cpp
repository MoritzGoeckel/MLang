#include <stdio.h>
#include <stdlib.h>

// To avoid renaming the function in the exported symbols
extern "C" {
    void print();
    int mul(int a, int b);
}

void print() { printf("It's working!\n"); }

int mul(int a, int b) {
    int result = a * b;
    printf("Multiplication result: %d\n", result);
    return result;
}