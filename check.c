/* 
 * Copyright 2019 Yuxuan Xiao

 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
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

TY_entry C_checkVar(A_var var, S_table venv, S_table tenv) {
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
            A_expList arrexp = var->u.arrayvar.exp;
            int level = 0;
            while (arrexp) {
                level++;
                arrexp = arrexp->tail;
            }
            if (arrayentry && arrayentry->kind == TY_ARRAY && level == arrayentry->u.array.level) {
                return arrayentry->u.array.type;
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
                ErrorMsg(var->linno, "%s is not a field of struct %s", S_Name(var->u.structvar.child),
                         S_Name(parent->u.struc.sname));
            }
            return child;
        }
    }
}

TY_entry C_checkDec(A_tyDec dec, S_symbol *name, S_table venv, S_table tenv) {
    switch (dec->kind) {
        case A_VAR_DEC: {
            TY_entry var = S_Find(tenv, dec->u.var.type);
            if (!var) {
                ErrorMsg(dec->linno, "unexpect type %s", S_Name(dec->u.var.type));
            }
            if (name) {
                S_symbol type = appendSymbol(*name, dec->u.var.name, PUNC);
                if (S_Find(tenv, type)) {
                    ErrorMsg(dec->linno, "duplicate struct field define %s", S_Name(dec->u.var.name));
                }
                S_Enter(tenv, type, var);
                name = &dec->u.var.name;
            } else {
                if (S_Find(venv, dec->u.var.name)) {
                    ErrorMsg(dec->linno, "duplicate var dec %s", S_Name(dec->u.var.name));
                }
                S_Enter(venv, dec->u.var.name, var);
            }
            return var;
        }
        case A_ARRAY_DEC: {
            if (S_Find(venv, dec->u.array.name)) {
                ErrorMsg(dec->linno, "duplicate array var %s", S_Name(dec->u.array.type));
            }
            TY_entry array = S_Find(tenv, dec->u.array.type);
            if (!array) {
                ErrorMsg(dec->linno, "unexpect array type %s", S_Name(dec->u.array.type));
            }
            A_expList list = dec->u.array.exp;
            int level = 0;
            while (list && list->head) {
                level++;
                if (list->head->kind != A_CONST && list->head->u.cons.kind != TY_INT) {
                    ErrorMsg(dec->linno, "array dec exp must be int");
                }
                list = list->tail;
            }
            TY_entry actTy = TY_Array(array, level);
            S_Enter(venv, dec->u.array.name, actTy);
            return actTy;
        }
        case A_NULL_DEC:
            return TY_Void();
    }
}

TY_entry C_checkExp(A_exp exp, S_table venv, S_table tenv) {
    switch (exp->kind) {
        case A_CONST: {
            switch (exp->u.cons.kind) {
                case A_INT:
                    return TY_Int();
                case A_CHAR:
                    return TY_Char();
                case A_FLOAT:
                    return TY_Float();
                case A_VAR:
                    return C_checkVar(exp->u.cons.u.var, venv, tenv);
            }
        }
        case A_CALL: {
            TY_entry funent = S_Find(venv, exp->u.call.name);
            if (!funent || funent->kind != TY_FUN) {
                ErrorMsg(exp->linno, "no function %s defined", S_Name(exp->u.call.name));
                return NULL;
            }
            A_expList callpara = exp->u.call.para;
            TY_entryList funpara = funent->u.fun.para;
            while (callpara && callpara->head
                   && funpara && funpara->head) {
                TY_entry paraent = C_checkExp(callpara->head, venv, tenv);
                if (!paraent) {
                    ErrorMsg(exp->linno, "call para exp error");
                    break;
                }
                if (!sameTy(paraent, funpara->head)) {
                    ErrorMsg(exp->linno, "call para type not match");
                    break;
                }
                callpara = callpara->tail;
                funpara = funpara->tail;
            }
            if (callpara || funpara) {
                ErrorMsg(exp->linno, "call para count not match");
            }
            return funent->u.fun.ret;
        }
        case A_DOUBLE_EXP: {
            TY_entry leftentry = C_checkExp(exp->u.doublexp.left, venv, tenv);
            TY_entry rightentry = C_checkExp(exp->u.doublexp.right, venv, tenv);
            if (!leftentry || !rightentry) {
                ErrorMsg(exp->linno, "exp error"); 
                return NULL;
            }
            if (leftentry->kind != TY_INT && leftentry->kind != TY_CHAR && leftentry->kind != TY_FLOAT
                && rightentry->kind != TY_INT && rightentry->kind != TY_CHAR && rightentry->kind != TY_FLOAT) {
                ErrorMsg(exp->linno, "algorithmic exp type error");
            }
            switch (exp->u.doublexp.op) {
                case A_PLUS:
                case A_MINUS:
                case A_TIMES:
                case A_DIVIDE:
                    if (leftentry->kind == TY_FLOAT || rightentry->kind == TY_FLOAT) {
                        return TY_Float();
                    } else if (leftentry->kind == TY_INT || rightentry->kind == TY_INT) {
                        return TY_Int();
                    } else {
                        return TY_Char();
                    }
                case A_EQ:
                case A_NEQ:
                case A_GT:
                case A_GE:
                case A_LT:
                case A_LE:
                    return TY_Int();
                case A_BITAND:
                case A_BITOR:
                case A_AND:
                case A_OR:
                    if (leftentry->kind == TY_FLOAT || rightentry->kind == TY_FLOAT) {
                        ErrorMsg(exp->linno, "logic algo exp can not has float type");
                        return TY_Int();
                    } else if (leftentry->kind == TY_INT || rightentry->kind == TY_INT) {
                        return TY_Int();
                    } else {
                        return TY_Char();
                    }
            }
        }
        case A_SINGLE_EXP: {
            TY_entry expentry = C_checkExp(exp->u.singexp.exp, venv, tenv);
            if (!expentry) {
                ErrorMsg(exp->linno, "exp error"); 
            }
            if (expentry->kind != TY_INT && expentry->kind != TY_CHAR) {
                ErrorMsg(exp->linno, "algorithmic exp type error"); 
            }
            switch (exp->u.singexp.op) {
                case A_SPLUS:
                case A_SMINUS:
                case A_NEGATIVE:
                case A_POSITIVE:
                case A_NOT:
                    return expentry;
            }
        }
    }
}

void C_checkStm(A_stm stm, S_table venv, S_table tenv) {
    switch (stm->kind) {
        case A_ASSIGN_STM: {
            TY_entry value = C_checkVar(stm->u.assign.symbol, venv, tenv);
            TY_entry expty = C_checkExp(stm->u.assign.exp, venv, tenv);
            if (!value || !expty) {
                break;
            }
            if (!sameTy(value, expty)) {
                ErrorMsg(stm->linno, "value type and exp type not match");
            }
            break;
        }
        case A_DEC_STM: {
            C_checkDec(stm->u.dec, NULL, venv, tenv);
            break;
        }
        case A_IF_STM: {
            TY_entry expty = C_checkExp(stm->u.iff.test, venv, tenv);
            if (expty->kind != TY_INT && expty->kind != TY_CHAR) {
                ErrorMsg(stm->linno, "unexpect if test exp type");
            }
            S_BeginScope(venv);
            A_stmList ifbody = stm->u.iff.iff;
            while (ifbody && ifbody->head) {
                C_checkStm(ifbody->head, venv, tenv);
                ifbody = ifbody->tail;
            }
            S_EndScope(venv);
            if (stm->u.iff.elsee) {
                A_stmList elsebody = stm->u.iff.elsee;
                S_BeginScope(venv);
                while (elsebody && elsebody->head) {
                    C_checkStm(elsebody->head, venv, tenv);
                    elsebody = elsebody->tail;
                }
                S_EndScope(venv);
            }
            break;
        }
        case A_WHILE_STM: {
            TY_entry expty = C_checkExp(stm->u.whilee.test, venv, tenv);
            if (expty->kind != TY_INT && expty->kind != TY_CHAR) {
                ErrorMsg(stm->linno, "unexpect while test exp type");
            }
            S_BeginScope(venv);
            A_stmList whilebody = stm->u.whilee.whilee;
            while (whilebody && whilebody->head) {
                C_checkStm(whilebody->head, venv, tenv);
                whilebody = whilebody->tail;
            }
            break;
        }
        case A_BREAK_STM:
        case A_CONTINUE_STM:
        case A_RETURN_STM:
            // break continue return 先不做检查
            break;
        case A_EXP_STM:
            break;
    }

}

void C_checkGlobalDec(A_globalDec globalDec, S_table venv, S_table tenv) {
    switch (globalDec->kind) {
        case A_FUN: {
            TY_entry entry = S_Find(venv, globalDec->u.fun.name);
            if (entry) {
                ErrorMsg(globalDec->linno, "duplicate global fun dec %s", S_Name(globalDec->u.fun.name));
            }
            S_BeginScope(tenv);
            S_BeginScope(venv);
            TY_entry ret = S_Find(tenv, globalDec->u.fun.ret);
            if (ret == NULL) {
                ErrorMsg(globalDec->linno, "unexpect function return type");
            }
            TY_entryList paraList = NULL;
            if (globalDec->u.fun.para && globalDec->u.fun.para->head->kind != A_NULL_DEC) {
                A_tyDecList paras = globalDec->u.fun.para;
                while (paras && paras->head) {
                    TY_entry funPara = C_checkDec(paras->head, NULL, venv, tenv);
                    if (!funPara) {
                        ErrorMsg(globalDec->linno, "unexpect function para type");
                    } else {
                        paraList = TY_EntryList(funPara, paraList);
                    }
                    paras = paras->tail;
                }
            }
            // revert list to keep paralist order
            TY_entryList revertParaList = paraList;
            if (paraList) {
                paraList = paraList->tail;
                revertParaList->tail = NULL;
                while (paraList) {
                    revertParaList = TY_EntryList(paraList->head, revertParaList);
                    paraList = paraList->tail;
                }
            }
            TY_entry funentry = TY_Fun(ret, revertParaList);
            S_Enter(venv, globalDec->u.fun.name, funentry);
            A_stmList list = globalDec->u.fun.body;
            while (list && list->head) {
                C_checkStm(list->head, venv, tenv);
                list = list->tail;
            }
            S_EndScope(venv);
            S_EndScope(tenv);
            S_Enter(venv, globalDec->u.fun.name, funentry);
            break;
        }
        case A_STRUCT: {
            TY_structDataList strList = NULL;
            TY_entry entry = S_Find(tenv, globalDec->u.struc.name);
            if (entry) {
                ErrorMsg(globalDec->linno, "duplicate global struct dec %s", S_Name(globalDec->u.struc.name));
            }
            A_tyDecList declist = globalDec->u.struc.declist;
            while (declist && declist->head) {
                S_symbol *para = &globalDec->u.struc.name;
                TY_entry decs = C_checkDec(declist->head, para, tenv, tenv);
                if (!decs) {
                    ErrorMsg(globalDec->linno, "unexpect struct type");
                } else {
                    strList = TY_StructDataList(TY_StructData(*para, decs), strList);
                }
                declist = declist->tail;
            }
            S_Enter(tenv, globalDec->u.struc.name, TY_Struct(globalDec->u.struc.name, strList));
            break;
        }
        case A_GLOBAL_VAR: {
            TY_entry entry = S_Find(venv, globalDec->u.var.name);
            if (entry) {
                ErrorMsg(globalDec->linno, "duplicate global dec %s", S_Name(globalDec->u.var.name));
            }
            entry = S_Find(tenv, globalDec->u.var.type);
            if (!entry) {
                ErrorMsg(globalDec->linno, "unexpect global dec type");
            }
            S_Enter(venv, globalDec->u.var.name, entry);
            break;
        }
    }
}


void C_checkGlobalDecList(A_globalDecList list) {
    S_table venv = E_BaseVarTable();
    S_table tenv = E_BaseTypeTable();
    while (list && list->head) {
        C_checkGlobalDec(list->head, venv, tenv);
        list = list->tail;
    }
}