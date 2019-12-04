#include "util.h"
#include "type.h"
#include "symbol.h"
#include "env.h"

S_table E_BaseVarTable() {
    S_table table = S_NewTable(TRUE);
    return table;
}

S_table E_BaseTypeTable() {
    S_table table = S_NewTable(FALSE);
    S_Enter(table, S_Symbol("void"), TY_Void());
    S_Enter(table, S_Symbol("int"), TY_Int());
    S_Enter(table, S_Symbol("float"), TY_Float());
    S_Enter(table, S_Symbol("char"), TY_Char());
    return table;
}