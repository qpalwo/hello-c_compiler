#include "util.h"
#include "absyn.h"

LIST_FUN(fun, Fun)
LIST_FUN(exp, Exp)
LIST_FUN(field, Field)
LIST_FUN(data, Data)
LIST_FUN(stm, Stm)

A_fun A_Fun(A_line linno, A_type ret, string name, A_fieldList para, A_expList body) {
    A_fun fun = checked_malloc(sizeof(*fun));
    fun->linno = linno;
    fun->ret = ret;
    fun->name = name;
    fun->para = para;
    fun->body = body;
    return fun;
}

A_type A_Void(A_line linno) {
    A_type ty = checked_malloc(sizeof(*ty));
    ty->linno = linno;
    ty->kind = A_VOID;
    return ty;
}

A_type A_Int(A_line linno) {
    A_type ty = checked_malloc(sizeof(*ty));
    ty->linno = linno;
    ty->kind = A_INT;
    return ty;
}

A_type A_Float(A_line linno) {
    A_type ty = checked_malloc(sizeof(*ty));
    ty->linno = linno;
    ty->kind = A_FLOAT;
    return ty;
}

A_type A_Symbol(A_line linno) {
    A_type ty = checked_malloc(sizeof(*ty));
    ty->linno = linno;
    ty->kind = A_SYMBOL;
    return ty;
}

A_data A_IData(A_line linno, int val) {
    A_data data = checked_malloc(sizeof(*data));
    data->type = A_Int(linno);
    data->u.ival = val;
    return data;
}

A_data A_FData(A_line linno, float val) {
    A_data data = checked_malloc(sizeof(*data));
    data->type = A_Float(linno);
    data->u.fval = val;
    return data;
}

A_data A_VData(A_line linno) {
    A_data data = checked_malloc(sizeof(*data));
    data->type = A_Void(linno);
    return data;
}

A_data A_SData(A_line linno, string symbol) {
    A_data data = checked_malloc(sizeof(*data));
    data->type = A_Symbol(linno);
    data->u.symbol = symbol;
    return data;
}

A_field A_Field(A_line linno, A_type type, string name) {
    A_field field = checked_malloc(sizeof(*field));
    field->linno = linno;
    field->type = type;
    field->name = name;
    return field;
}

A_stm A_Assign(A_line linno, string field, A_exp exp) {
    A_stm assign = checked_malloc(sizeof(*exp));
    assign->linno = linno;
    assign->kind = A_ASSIGN;
    assign->u.assign.symbol = field;
    assign->u.assign.exp = exp;
    return exp;
}

A_exp A_Call(A_line linno, string name, A_expList para) {
    A_exp exp = checked_malloc(sizeof(*exp));
    exp->linno = linno;
    exp->kind = A_CALL;
    exp->u.call.name = name;
    exp->u.call.para = para;
    return exp;
}