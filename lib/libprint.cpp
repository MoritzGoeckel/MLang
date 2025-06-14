#include <stdio.h>
#include <stdlib.h>

// To avoid renaming the function in the exported symbols
extern "C" {
    void print();
    int mul(int a, int b);
    void test_v();
    int test_ii_i(int a, int b);
    int test_iii_i(int a, int b, int c);
    int test_iiii_i(int a, int b, int c, int d);
    int test_iiiii_i(int a, int b, int c, int d, int e);
    int test_iiiiii_i(int a, int b, int c, int d, int e, int f);
    bool test_ii_b(int a, int b);
    bool test_bb_b(bool a, bool b);
}

void print() { printf("It's working!\n"); }

void test_v() {
    printf("Test: void()\n");
}

int test_ii_i(int a, int b) {
    int r = a + b;
    printf("Test (%d, %d)\n", a, b);
    return r;
}


int test_iii_i(int a, int b, int c) {
    int r = a + b + c;
    printf("Test (%d, %d, %d)\n", a, b, c);
    return r;
}


int test_iiii_i(int a, int b, int c, int d) {
    int r = a + b + c + d;
    printf("Test (%d, %d, %d, %d)\n", a, b, c, d);
    return r;
}

int test_iiiii_i(int a, int b, int c, int d, int e) {
    int r = a + b + c + d + e;
    printf("Test (%d, %d, %d, %d, %d)\n", a, b, c, d, e);
    return r;
}

int test_iiiiii_i(int a, int b, int c, int d, int e, int f) {
    int r = a + b + c + d + e + f;
    printf("Test (%d, %d, %d, %d, %d, %d)\n", a, b, c, d, e, f);
    return r;
}

bool test_ii_b(int a, int b) {
    bool r = a == b;
    printf("Test (%d, %d) -> %s\n", a, b, r ? "true" : "false");
    return r;
}

bool test_bb_b(bool a, bool b) {
    bool r = a == b;
    printf("Test (%s, %s) -> %s\n", a ? "true" : "false", b ? "true" : "false", r ? "true" : "false");
    return r;
}