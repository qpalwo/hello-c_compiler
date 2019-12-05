#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "codegen.h"
#include "astprinter.h"
#include "check.h"

extern FILE *yyin;
extern int yyparse(void);
extern int yylex(void);
extern A_globalDecList absyn_root;
extern int yydebug;
extern int lexPrint;
extern int rootnull;
extern void printSymbol(FILE * out, int index);
int printWord = 0;
int pAST = 0;
int checkAST = 0;
int CODEGEN_DEBUG = 1;
int TABLE_DEBUG = 1;

void parse(string fileName) {
    yyin = fopen(fileName, "r");
    if (!yyin) {
        printf("open file error");
        exit(0);
    }
    yyparse();
}

int main(int argc, char **argv) {
    int opt;
    string option = "cwdhpf:t";
    string filePath = NULL;
    while ((opt = getopt(argc, argv, option)) != -1) {
        if (opt == 'h') {
            printf("-f <path-to-file>\n");
            printf("-t for lex print\n");
            printf("-d for bison debug output\n");
            printf("-w for word \n -p for AST print \n -h for this help");
            return 0;
        }
        if (opt == 'd') {
            yydebug = 1;
        }
        if (opt == 't') {
            lexPrint = 1;
        }
        if (opt == 'f') {
            filePath = optarg;
        }
        if (opt == 'w') {
            printWord = 1;
        }
        if (opt == 'p') {
            pAST = 1;
        }
        if (opt == 'c') {
            checkAST = 1;
        }
    }
    if (!filePath) {
        printf("-f MUST BE ASSIGNED");
        return 1;
    }
    if (printWord) {
        int num = 0;
        yyin = fopen(filePath, "r");
        while ((num = yylex()) > 0) {
            printf("%d\t", num);
            printSymbol(stdout, num);
            printf("\n");
        }
        return 0;
    }
    parse(filePath);
    if (pAST) {
        printAST(stdout, absyn_root);
    }
    if (checkAST) {
        C_checkGlobalDecList(absyn_root);
    }
    CG_codeGen(absyn_root);
    return 0;
}