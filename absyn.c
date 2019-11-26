#include "util.h"
#include "absyn.h"

LIST_FUN(fun, Fun)
LIST_FUN(exp, Exp)
LIST_FUN(field, Field)



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

A_data A_IData(A_line linno, int val) {
    A_data data = checked_malloc(sizeof(*data));
    data->type = A_Int(linno);
    data->linno = linno;
    data->u.ival = val;
    return data;
}

A_data A_FData(A_line linno, float val) {
    A_data data = checked_malloc(sizeof(*data));
    data->type = A_Float(linno);
    data->linno = linno;
    data->u.fval = val;
    return data;
}

A_data A_VData(A_line linno) {
    A_data data = checked_malloc(sizeof(*data));
    data->type = A_Void(linno);
    data->linno = linno;
    return data;
}

A_field A_Field(A_line linno, A_type type, string name) {
    A_field field = checked_malloc(sizeof(*field));
    field->linno = linno;
    field->type = type;
    field->name = name;
    return field;
}


// A_fieldList A_FieldList(A_field head, A_fieldList tail) {
//     A_fieldList list = checked_malloc(sizeof(*list));
//     list->head = head;
//     list->tail = tail;
//     return list;
// }

// A_expList A_ExpList(A_exp head, A_expList tail) {
//     A_expList list = checked_malloc(sizeof(*list));
//     list->head = head;
//     list->tail = tail;
//     return tail;
// }

// A_funList A_FunList(A_fun head, A_funList tail) {

// }
