#include "util.h"
#include "symbol.h"
#include "absyn.h"

LIST_FUN(exp, Exp)
LIST_FUN(stm, Stm)
LIST_FUN(globalDec, GlobalDec)
LIST_FUN(tyDec, TyDec)

A_globalDec A_Fun(A_line linno, S_symbol ret, S_symbol name, A_tyDecList para, A_expList body) {
    A_globalDec fun = checked_malloc(sizeof(*fun));
    fun->u.fun.linno = linno;
    fun->u.fun.ret = ret;
    fun->u.fun.name = name;
    fun->u.fun.para = para;
    fun->u.fun.body = body;
    return fun;
}

A_tyDec A_Var(A_line linno, S_symbol type, S_symbol name) {
    A_tyDec tydec = checked_malloc(sizeof(*tydec));
    tydec->kind = A_VAR;
    tydec->linno = linno;
    tydec->u.var.type = type;
    tydec->u.var.name = name;
    return tydec;
}

A_globalDec A_Struct(A_line linno, S_symbol name, A_tyDecList declist) {
    A_globalDec tydec = checked_malloc(sizeof(*tydec));
    tydec->kind = A_STRUCT;
    tydec->linno = linno;
    tydec->u.struc.name = name;
    tydec->u.struc.declist = declist;
    return tydec;
}

A_stm A_Assign(A_line linno, string field, A_exp exp) {
    A_stm assign = checked_malloc(sizeof(*exp));
    assign->linno = linno;
    assign->kind = A_ASSIGN;
    assign->u.assign.symbol = field;
    assign->u.assign.exp = exp;
    return assign;
}

A_exp A_Call(A_line linno, string name, A_expList para) {
    A_exp exp = checked_malloc(sizeof(*exp));
    exp->linno = linno;
    exp->kind = A_CALL;
    exp->u.call.name = name;
    exp->u.call.para = para;
    return exp;
}