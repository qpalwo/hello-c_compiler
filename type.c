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
#include "util.h"
#include "symbol.h"
#include "type.h"

struct TY_entry_ voidd = { TY_VOID };

TY_entry TY_Void() {
    return &voidd;
}

struct TY_entry_ intt = { TY_INT };

TY_entry TY_Int() {
    return &intt;
}

struct TY_entry_ floatt = { TY_FLOAT };

TY_entry TY_Float() {
    return &floatt;
}

struct TY_entry_ charr = { TY_CHAR };

TY_entry TY_Char() {
    return &charr;
}

TY_entry TY_Array(TY_entry type, int level) {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_ARRAY;
    entry->u.array.type = type;
    entry->u.array.level = level;
    return entry;
}

TY_entry TY_Struct(S_symbol name, TY_structDataList types) {
    TY_entry entry = checked_malloc(sizeof(*entry));
    entry->kind = TY_STRUCT;
    entry->u.struc.sname = name;
    entry->u.struc.sdl = types;
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

TY_structData TY_StructData(S_symbol name, TY_entry type) {
    TY_structData data = checked_malloc(sizeof(*data));
    data->name = name;
    data->type = type;
    return data;
}

TY_structDataList TY_StructDataList(TY_structData head, TY_structDataList tail) {
    TY_structDataList list = checked_malloc(sizeof(*list));
    list->head = head;
    list->tail = tail;
    return list;
}