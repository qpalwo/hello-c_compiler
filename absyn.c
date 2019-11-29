#include "util.h"
#include "symbol.h"
#include "absyn.h"

LIST_FUN(exp, Exp)
LIST_FUN(stm, Stm)
LIST_FUN(globalDec, GlobalDec)
LIST_FUN(tyDec, TyDec)

A_globalDec A_Fun(A_line linno, S_symbol ret, S_symbol name, A_tyDecList para, A_stmList body) {
    A_globalDec fun = checked_malloc(sizeof(*fun));
    fun->u.fun.linno = linno;
    fun->u.fun.ret = ret;
    fun->u.fun.name = name;
    fun->u.fun.para = para;
    fun->u.fun.body = body;
    return fun;
}

A_tyDec A_VarDec(A_line linno, S_symbol type, S_symbol name) {
    A_tyDec tydec = checked_malloc(sizeof(*tydec));
    tydec->kind = A_VAR_DEC;
    tydec->linno = linno;
    tydec->u.var.type = type;
    tydec->u.var.name = name;
    return tydec;
}

A_tyDec A_ArrayDec(A_line linno, S_symbol type, S_symbol name, A_expList exp) {
    A_tyDec tydec = checked_malloc(sizeof(*tydec));
    tydec->kind = A_ARRAY_DEC;
    tydec->linno = linno;
    tydec->u.array.name = name;
    tydec->u.array.type = type;
    tydec->u.array.exp = exp;
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

A_globalDec A_GlobalVar(A_line linno, S_symbol type, S_symbol name) {
    A_globalDec vardec = checked_malloc(sizeof(*vardec));
    vardec->kind = A_GLOBAL_VAR;
    vardec->linno = linno;
    vardec->u.var.name = name;
    vardec->u.var.type = type;
    return vardec;
}

A_var A_SymbolVar(A_line linno, S_symbol symbol) {
    A_var var = checked_malloc(sizeof(*var));
    var->linno = linno;
    var->kind = A_SYMBOL_VAR;
    var->u.symbol = symbol;
    return var;
}

A_var A_ArrayVar(A_line linno, S_symbol symbol, A_expList explist) {
    A_var var = checked_malloc(sizeof(*var));
    var->linno = linno;
    var->kind = A_ARRAY_VAR;
    var->u.arrayvar.symbol = symbol;
    var->u.arrayvar.exp = explist;
    return var;
}

A_var A_StructVar(A_line linno, S_symbol para, S_symbol child) {
    A_var var = checked_malloc(sizeof(*var));
    var->linno = linno;
    var->kind = A_STRUCT_VAR;
    var->u.structvar.para = para;
    var->u.structvar.child = child;
    return var;
}

A_stm A_AssignStm(A_line linno, A_var field, A_exp exp) {
    A_stm assign = checked_malloc(sizeof(*exp));
    assign->linno = linno;
    assign->kind = A_ASSIGN_STM;
    assign->u.assign.symbol = field;
    assign->u.assign.exp = exp;
    return assign;
}

A_stm A_DecStm(A_line linno, A_tyDec tydec) {
    A_stm dec = checked_malloc(sizeof(*dec));
    dec->linno = linno;
    dec->kind = A_DEC_STM;
    dec->u.dec = tydec;
    return dec;
}

A_stm A_IfStm(A_line linno, A_exp test, A_stmList iff, A_stmList elsee) {
    A_stm ifstm = checked_malloc(sizeof(*ifstm));
    ifstm->linno = linno;
    ifstm->kind = A_IF_STM;
    ifstm->u.iff.test = test;
    ifstm->u.iff.iff = iff;
    ifstm->u.iff.elsee = elsee;
    return ifstm;
}

A_stm A_WhileStm(A_line linno, A_exp test, A_stmList whilee) {
    A_stm whilestm = checked_malloc(sizeof(*whilestm));
    whilestm->linno = linno;
    whilestm->kind = A_WHILE_STM;
    whilestm->u.whilee.test = test;
    whilestm->u.whilee.whilee = whilee;
    return whilestm;
}

A_stm A_BreakStm(A_line linno) {
    A_stm breakstm = checked_malloc(sizeof(*breakstm));
    breakstm->linno = linno;
    breakstm->kind = A_BREAK_STM;
    return breakstm;
}

A_stm A_ContinueStm(A_line linno) {
    A_stm continuestm = checked_malloc(sizeof(*continuestm));
    continuestm->linno = linno;
    continuestm->kind = A_CONTINUE_STM;
    return continuestm;
}

A_stm A_ReturnStm(A_line linno, A_exp ret) {
    A_stm returnstm = checked_malloc(sizeof(*returnstm));
    returnstm->linno = linno;
    returnstm->kind = A_RETURN_STM;
    returnstm->u.returnn = ret;
    return returnstm;
}

A_exp A_Call(A_line linno, S_symbol name, A_expList para) {
    A_exp exp = checked_malloc(sizeof(*exp));
    exp->linno = linno;
    exp->kind = A_CALL;
    exp->u.call.name = name;
    exp->u.call.para = para;
    return exp;
}

A_exp A_SingleExp(A_line linno, sop sop, A_exp exp) {
    A_exp singexp = checked_malloc(sizeof(*singexp));
    singexp->linno = linno;
    singexp->kind = A_SINGLE_EXP;
    singexp->u.singexp.op = sop;
    singexp->u.singexp.exp = exp;
    return singexp;
}

A_exp A_DoubleExp(A_line linno, dop dop, A_exp leftExp, A_exp rightExp) {
    A_exp doublexp = checked_malloc(sizeof(*doublexp));
    doublexp->linno = linno;
    doublexp->kind = A_DOUBLE_EXP;
    doublexp->u.doublexp.op = dop;
    doublexp->u.doublexp.left = leftExp;
    doublexp->u.doublexp.right = rightExp;
    return doublexp;
}

A_exp A_Char(A_line linnno, char chr) {
    A_exp exp = checked_malloc(sizeof(*exp));
    exp->linno = linnno;
    exp->kind = A_CONST;
    exp->u.cons.kind = A_CHAR;
    exp->u.cons.u.cnum = chr;
    return exp;
}

A_exp A_Int(A_line linnno, int inum) {
    A_exp exp = checked_malloc(sizeof(*exp));
    exp->linno = linnno;
    exp->kind = A_CONST;
    exp->u.cons.kind = A_INT;
    exp->u.cons.u.inum = inum;
    return exp;
};

A_exp A_Float(A_line linno, float fnum) {
    A_exp exp = checked_malloc(sizeof(*exp));
    exp->linno = linno;
    exp->kind = A_CONST;
    exp->u.cons.kind = A_FLOAT;
    exp->u.cons.u.fnum = fnum;
    return exp;
}

A_exp A_VarExp(A_line linno, A_var var) {
    A_exp exp = checked_malloc(sizeof(*exp));
    exp->linno = linno;
    exp->kind = A_CONST;
    exp->u.cons.kind = A_VAR;
    exp->u.cons.u.var = var;
    return exp;
}