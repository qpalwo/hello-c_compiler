#include <stdio.h>
#include <stdlib.h>
#include "util.h"

extern FILE *yyin;
extern int yyparse(void);

void parse(string fileName) {
    yyin = fopen(fileName, "r");
    if (!yyin) {
        printf("open file error");
        exit(0);
    }
    yyparse();
}

int main(int argc, char **argv) {
    parse(argv[1]);
    return 0;
}