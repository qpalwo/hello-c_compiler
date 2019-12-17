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
typedef struct TY_entry_ * TY_entry;
typedef struct TY_entryList_ * TY_entryList;
typedef struct TY_structData_ * TY_structData;
typedef struct TY_structDataList_ * TY_structDataList;
typedef struct S_symbol_ * S_symbol;

struct TY_entryList_ {
    TY_entry head;
    TY_entryList tail;
};

struct TY_structDataList_ {
    TY_structData head;
    TY_structDataList tail;
};

struct TY_structData_ {
    S_symbol name;
    TY_entry type;
};


struct TY_entry_ {
    enum {
        TY_VOID,
        TY_INT,
        TY_FLOAT,
        TY_CHAR,
        TY_ARRAY,
        TY_STRUCT,
        TY_FUN
    } kind;
    union {
        struct {
            TY_entry type;
            int level;
        } array;
        struct {
            TY_structDataList sdl;
            S_symbol sname;
        } struc;
        struct {
            TY_entry ret;
            TY_entryList para;
        } fun;
    } u;
};

TY_entry TY_Void();

TY_entry TY_Int();

TY_entry TY_Float();

TY_entry TY_Char();

TY_entry TY_Array(TY_entry type, int level);

TY_entry TY_Struct(S_symbol name, TY_structDataList types);

TY_entry TY_Fun(TY_entry ret, TY_entryList paras);

TY_entryList TY_EntryList(TY_entry head, TY_entryList tail);

TY_structData TY_StructData(S_symbol name, TY_entry type);

TY_structDataList TY_StructDataList(TY_structData head, TY_structDataList tail);