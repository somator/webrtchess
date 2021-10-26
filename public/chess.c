// hello.c
#include <stdio.h>
#include "emscripten.h"

int version() {
    return 1;
}

char *hello_world() {
    return "hello, world!\n";
}