#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "check.h"
#include "env.h"

void checkGlobalDec(A_globalDec globalDec, S_table fenv, S_table tenv) {
    switch (globalDec->kind) {
        case A_FUN: {
            S_BeginScope(tenv);
            S_EndScope(tenv);
            break;
        }
        case A_STRUCT: {
            break;
        }
        case A_GLOBAL_VAR: {
            break;
        }
    }
}


void checkGlobalDecList(A_globalDecList list) {
    while (list && list->head) {
        checkGlobalDec(list->head, E_BaseFunTable(), E_BaseTypeTable());
        list = list->tail;
    }
}