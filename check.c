#include "util.h"
#include "type.h"
#include "symbol.h"
#include "absyn.h"
#include "check.h"
#include "env.h"
#define NULL 0
#define FOERACH_LIST(name, action) \
while (name && name##->head) { \
    action \
    name = name->tail; \
}


TY_entry checkDec(A_tyDec dec, S_table venv, S_table tenv) {
    switch (dec->kind) {
        case A_VAR_DEC: {
            TY_entry var = S_Find(tenv, dec->u.var.type);
            if (!var) {
                ErrorMsg(dec->linno, "unexpect type %s", S_name(dec->u.var.type));
            }
            S_Enter(venv, dec->u.var.name, var);
            return var;
        }
        case A_ARRAY_DEC: {
            TY_entry array = S_Find(tenv, dec->u.array.type);
            if (!array) {
                ErrorMsg(dec->linno, "unexpect array type %s", S_name(dec->u.array.type));
            }
            A_expList list = dec->u.array.exp;
            int level = 0;
            while (list && list->head) {
                level++;
                list = list->tail;
            }
            TY_entry actTy = TY_Array(array, level);
            S_Enter(venv, dec->u.array.name, actTy);
            return actTy;
        }
    }
}

void checkStm(A_stm stm, S_table venv, S_table tenv) {

}

void checkGlobalDec(A_globalDec globalDec, S_table venv, S_table tenv) {
    switch (globalDec->kind) {
        case A_FUN: {
            S_BeginScope(tenv);
            S_BeginScope(venv);
            TY_entry ret = S_Find(tenv, globalDec->u.fun.ret);
            if (ret == NULL) {
                ErrorMsg(globalDec->linno, "unexpect function return type");
            }
            TY_entryList paraList = NULL;
            if (globalDec->u.fun.para) {
                A_tyDecList paras = globalDec->u.fun.para;
                while (paras && paras->head) {
                    TY_entry funPara = checkDec(paras->head, venv, tenv);
                    if (!funPara) {
                        ErrorMsg(globalDec->linno, "unexpect function para type");
                    } else {
                        paraList = TY_EntryList(funPara, paraList);
                    }
                }
            }
            S_Enter(venv, globalDec->u.fun.name, TY_Fun(ret, paraList));
            A_stmList list = globalDec->u.fun.body;
            while (list && list->head) {
                checkStm(list->head, venv, tenv);
                list = list->tail;
            }
            S_EndScope(venv);
            S_EndScope(tenv);
            break;
        }
        case A_STRUCT: {
            TY_entryList strList = NULL;
            A_tyDecList declist = globalDec->u.struc.declist;
            while (declist && declist->head) {
                TY_entry funPara = checkDec(declist->head, tenv, tenv);
                if (!funPara) {
                    ErrorMsg(globalDec->linno, "unexpect struct type");
                } else {
                    strList = TY_EntryList(funPara, strList);
                }
            }
            S_Enter(tenv, globalDec->u.struc.name, TY_Struct(strList));
            break;
        }
        case A_GLOBAL_VAR: {
            TY_entry entry = S_Find(tenv, globalDec->u.var.type);
            if (!entry) {
                ErrorMsg(globalDec->linno, "unexpect global dec type");
            }
            S_Enter(venv, globalDec->u.var.name, entry);
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