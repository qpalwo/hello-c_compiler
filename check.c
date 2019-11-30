#include <assert.h>
#include <string.h>
#include "util.h"
#include "type.h"
#include "symbol.h"
#include "absyn.h"
#include "check.h"
#include "env.h"

const string PUNC = ".";

S_symbol appendSymbol(S_symbol s1, S_symbol s2, const string punc) {
    string str1 = S_Name(s1);
    string str2 = S_Name(s2);
    int strl1 = strlen(str1);
    int strl2 = strlen(str2);
    int puncl = strlen(punc);
    int total = strl1 + strl2 + puncl + 1;
    string appended = checked_malloc(total);
    memset(appended, 0, total);
    memcpy(appended, str1, strl1);
    memcpy(appended + strl1, punc, puncl);
    memcpy(appended + strl1 + puncl, str2, strl2);
    return S_Symbol(appended);
}

bool sameTy(TY_entry one, TY_entry other) {
    assert(one && other);
    if (one->kind != other->kind) {
        return FALSE;
    }
    switch (one->kind) {
        case TY_VOID:
        case TY_INT:
        case TY_FLOAT:
        case TY_CHAR:
            return TRUE;
        case TY_ARRAY: 
            return sameTy(one->u.array.type, other->u.array.type) 
            && one->u.array.level == other->u.array.level;
        case TY_STRUCT: {
            return one == other;
            // TY_structDataList onelist = one->u.struc.sdl;
            // TY_structDataList otherlist = other->u.struc.sdl;
            // while (onelist && onelist->head
            // && otherlist && otherlist->head) {
            //     if (!sameTy(onelist->head->type, otherlist->head->type)) {
            //         return FALSE;
            //     }
            //     onelist = onelist->tail;
            //     otherlist = otherlist->tail;
            // }
            // return TRUE;
        }
        case TY_FUN: 
            return sameTy(one->u.fun.ret, other->u.fun.ret);
    }

}

TY_entry checkVar(A_var var, S_table venv, S_table tenv) {
    switch (var->kind) {
        case A_SYMBOL_VAR: {
            TY_entry varentry = S_Find(venv, var->u.symbol);
            if (!varentry) {
                ErrorMsg(var->linno, "no var %s defined", S_Name(var->u.symbol));
            }
            return varentry;
        }
        case A_ARRAY_VAR: {
            TY_entry arrayentry = S_Find(venv, var->u.arrayvar.symbol);
            if (!arrayentry) {
                ErrorMsg(var->linno, "no var %s defined", S_Name(var->u.arrayvar.symbol));
            }
            return arrayentry;
        }
        case A_STRUCT_VAR: {
            TY_entry parent = S_Find(venv, var->u.structvar.para);
            if (!parent) {
                ErrorMsg(var->linno, "no var %s defined", S_Name(var->u.structvar.para));
                return parent;
            }
            if (parent->kind != TY_STRUCT) {
                ErrorMsg(var->linno, "var %s is not a struct", S_Name(var->u.structvar.para));
                return parent;
            }
            TY_entry child = S_Find(tenv, appendSymbol(parent->u.struc.sname, var->u.structvar.child, PUNC));
            if (!child) {
                ErrorMsg(var->linno, "%s is not a field of struct %s", S_Name(var->u.structvar.child), S_Name(parent->u.struc.sname));
            }
            return child;
        }
    }
}

TY_entry checkDec(A_tyDec dec, S_symbol * name, S_table venv, S_table tenv) {
    switch (dec->kind) {
        case A_VAR_DEC: {
            TY_entry var = S_Find(tenv, dec->u.var.type);
            if (!var) {
                ErrorMsg(dec->linno, "unexpect type %s", S_Name(dec->u.var.type));
            }
            if (name) {
                S_Enter(tenv, appendSymbol(*name, dec->u.var.name, PUNC), var);
                name = &dec->u.var.name;
            } else {
                S_Enter(venv, dec->u.var.name, var);
            }
            return var;
        }
        case A_ARRAY_DEC: {
            TY_entry array = S_Find(tenv, dec->u.array.type);
            if (!array) {
                ErrorMsg(dec->linno, "unexpect array type %s", S_Name(dec->u.array.type));
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

TY_entry checkExp(A_exp exp, S_table venv, S_table tenv) {
    switch (exp->kind) {

    }

}

void checkStm(A_stm stm, S_table venv, S_table tenv) {
    switch (stm->kind) {
        case A_ASSIGN_STM: {
            TY_entry value = checkVar(stm->u.assign.symbol, venv, tenv);
            TY_entry expty = checkExp(stm->u.assign.exp, venv, tenv);
            if (!value || !expty) {
                break;
            }
            if (!sameTy(value, expty)) {
                ErrorMsg(stm->linno, "value type and exp type not match");
            }
            break;
        }
        case A_DEC_STM: {
            checkDec(stm->u.dec, NULL, venv, tenv);
            break;
        }
        case A_IF_STM: {
            TY_entry expty = checkExp(stm->u.iff.test, venv, tenv);
            if (expty->kind != TY_INT && expty->kind != TY_CHAR) {
                ErrorMsg(stm->linno, "unexpect if test exp type");
            }
            S_BeginScope(venv);
            A_stmList ifbody = stm->u.iff.iff;
            while (ifbody && ifbody->head) {
                checkStm(ifbody->head, venv, tenv);
                ifbody = ifbody->tail;
            }
            S_EndScope(venv);
            if (stm->u.iff.elsee) {
                A_stmList elsebody = stm->u.iff.elsee;
                S_BeginScope(venv);
                while (elsebody && elsebody->head) {
                    checkStm(elsebody->head, venv, tenv);
                    elsebody = elsebody->tail;
                }
                S_EndScope(venv);
            }
            break;
        }
        case A_WHILE_STM: {
            TY_entry expty = checkExp(stm->u.whilee.test, venv, tenv);
            if (expty->kind != TY_INT && expty->kind != TY_CHAR) {
                ErrorMsg(stm->linno, "unexpect while test exp type");
            }
            S_BeginScope(venv);
            A_stmList whilebody = stm->u.whilee.whilee;
            while (whilebody && whilebody->head) {
                checkStm(whilebody->head, venv, tenv);
                whilebody = whilebody->tail;
            }
            break;
        }
        case A_BREAK_STM:
        case A_CONTINUE_STM:
        case A_RETURN_STM:
            // break continue return 先不做检查
            break;
    }

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
                    TY_entry funPara = checkDec(paras->head, NULL, venv, tenv);
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
            TY_structDataList strList = NULL;
            A_tyDecList declist = globalDec->u.struc.declist;
            while (declist && declist->head) {
                S_symbol * para = &globalDec->u.struc.name;
                TY_entry decs = checkDec(declist->head, para, tenv, tenv);
                if (!decs) {
                    ErrorMsg(globalDec->linno, "unexpect struct type");
                } else {
                    strList = TY_StructDataList(TY_StructData(*para, decs), strList);
                }
            }
            S_Enter(tenv, globalDec->u.struc.name, TY_Struct(globalDec->u.struc.name, strList));
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
        checkGlobalDec(list->head, E_BaseVarTable(), E_BaseTypeTable());
        list = list->tail;
    }
}