#include "util.h"
#include "type.h"

TY_entry TY_Void() {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_VOID;
    return entry;
}

TY_entry TY_Int() {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_INT;
    return entry;
}

TY_entry TY_Float() {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_FLOAT;
    return entry;
}

TY_entry TY_Char() {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_CHAR;
    return entry;
}

TY_entry TY_Array(TY_entry type, int level) {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_ARRAY;
    entry->u.array.type = type;
    entry->u.array.level = level;
    return entry;
}

TY_entry TY_Struct(TY_entryList types) {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_STRUCT;
    entry->u.struc = types;
    return entry;
}

TY_entry TY_Fun(TY_entry ret, TY_entryList paras) {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_FUN;
    entry->u.fun.ret = ret;
    entry->u.fun.para = paras;
    return entry;
}

TY_entryList TY_EntryList(TY_entry head, TY_entryList tail) {
    TY_entryList list = checked_malloc(sizeof(*list));
    list->head = head;
    list->tail = tail;
    return list;
}