#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "astprinter.h"

static void indent(FILE * out, int deep) {
    for (int i = 0; i < deep; i++) {
        fprintf(out, "  ");
    }
}

void printExp(FILE * out, A_exp exp, int deepth) {
    if (!exp)
        return;
    indent(out, deepth + 1);
    switch (exp->kind) {
        case A_CONST:
            fprintf(out, "constExp\n");
            break;
        case A_CALL:
            fprintf(out, "callExp\n");
            printExpList(out, exp->u.call.para, deepth + 1);
            break;
        case A_DOUBLE_EXP:
            fprintf(out, "doubleExp\n");
            printExp(out, exp->u.doublexp.left, deepth + 1);
            printExp(out, exp->u.doublexp.right, deepth + 1);
            break;
        case A_SINGLE_EXP:
            fprintf(out, "singleExp\n");
            printExp(out, exp->u.singexp.exp, deepth + 1);
            break;
    }
}

void printExpList(FILE * out, A_expList root, int deepth) {
    if (!root)
        return;
    indent(out, deepth + 1);
    fprintf(out, "expList\n");
    while (root && root->head) {
        printExp(out, root->head, deepth + 1);
        root = root->tail;
    }
}

void printVar(FILE * out, A_var var, int deepth) {
    if (!var)
        return;
    indent(out, deepth + 1);
    switch (var->kind) {
        case A_SYMBOL_VAR:
            fprintf(out, "symbolVar\n");
            break;
        case A_ARRAY_VAR:
            fprintf(out, "arrayVar\n");
            printExpList(out, var->u.arrayvar.exp, deepth + 1);
            break;
        case A_STRUCT_VAR:
            fprintf(out, "structVar\n");
            break;
    }

}

void printTypeDec(FILE * out, A_tyDec dec, int deepth) {
    if (!dec)
        return;
    indent(out, deepth + 1);
    switch (dec->kind) {
        case A_VAR_DEC:
            fprintf(out, "varDec\n");
            break;
        case A_ARRAY_DEC:
            fprintf(out, "arrayDec\n");
            printExpList(out, dec->u.array.exp, deepth + 1);
            break;
    }
}

void printTypeDecList(FILE * out, A_tyDecList root, int deepth) {
    if (!root)
        return;
    indent(out, deepth + 1);
    fprintf(out, "typeDecList\n");
    while (root && root->head) {
        printTypeDec(out, root->head, deepth + 1);
        root = root->tail;
    }
}

void printStm(FILE * out, A_stm stm, int deepth) {
    if (!stm) 
        return;
    indent(out, deepth + 1);
    switch (stm->kind) {
        case A_ASSIGN_STM:
            fprintf(out, "assignStm\n");
            printVar(out, stm->u.assign.symbol, deepth + 1);
            printExp(out, stm->u.assign.exp, deepth + 1);
            break;
        case A_DEC_STM:
            fprintf(out, "decStm\n");
            printTypeDec(out, stm->u.dec, deepth + 1);
            break;
        case A_IF_STM:
            fprintf(out, "ifStm\n");
            printExp(out, stm->u.iff.test, deepth + 1);
            printStmList(out, stm->u.iff.iff, deepth + 1);
            printStmList(out, stm->u.iff.elsee, deepth + 1);
            break;
        case A_WHILE_STM:
            fprintf(out, "whileStm\n");
            printExp(out, stm->u.whilee.test, deepth + 1);
            printStmList(out, stm->u.whilee.whilee, deepth + 1);
            break;
        case A_BREAK_STM:
            fprintf(out, "breakStm\n");
            break;
        case A_CONTINUE_STM:
            fprintf(out, "continueStm\n");
            break;
        case A_RETURN_STM:
            fprintf(out, "returnStm\n");
            printExp(out, stm->u.returnn, deepth + 1);
            break;
    }
}

void printStmList(FILE * out, A_stmList root, int deepth) {
    if (!root)
        return;
    indent(out, deepth + 1);
    fprintf(out, "stmList\n");
    while (root && root->head) {
        printStm(out, root->head, deepth + 1);
        root = root->tail;
    }
}

void printGlobalDec(FILE * out, A_globalDec dec, int deepth) {
    if (!dec)
        return;
    indent(out, deepth + 1);
    switch (dec->kind) {
        case A_FUN:
            fprintf(out, "globalFunDec\n");
            printTypeDecList(out, dec->u.fun.para, deepth + 1);
            printStmList(out, dec->u.fun.body, deepth + 1);
            break;
        case A_STRUCT:
            fprintf(out, "globalStructDec\n");
            printTypeDecList(out, dec->u.struc.declist, deepth + 1);
            break;
        case A_GLOBAL_VAR:
            fprintf(out, "globalVarDec\n");
            break;
    }
}

void printAST(FILE * out, A_globalDecList root) {
    if (!root)
        return;
    fprintf(out, "globalDecList\n");
    while (root && root->head) {
        printGlobalDec(out, root->head, 0);
        root = root->tail;
    }
}