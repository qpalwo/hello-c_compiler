#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "util.h"

void *checked_malloc(int len) { 
    void *p = malloc(len);
    if (!p) {
        fprintf(stderr, "\nRan out of memory!\n");
        exit(1);
    } 
    return p;
}

string String(char *s) {
    string p = checked_malloc(strlen(s) + 1);
    strcpy(p, s);
    return p;
}

void ErrorMsg(int pos, char * message,...) {
    va_list ap;
    fprintf(stderr,"line %d: ", pos);
    va_start(ap, message);
    vfprintf(stderr, message, ap);
    va_end(ap);
    fprintf(stderr, "\n");
}