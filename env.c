#include "util.h"
#include "symbol.h"
#include "env.h"

S_table E_BaseFunTable() {
    S_table table = S_NewTable();
    return table;
}

S_table E_BaseTypeTable() {
    S_table table = S_NewTable();
    return table;
}