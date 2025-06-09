#include <stdio.h>
#include <stdlib.h>

// To avoid renaming the function in the exported symbols
extern "C" {
    void myprint();
}

void myprint() { printf("It's working!\n"); }
