#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "label.h"
#define DEFAULT_COUNT_SIZE 10

static string BBNAME = "<entry>";
static string DEFAULT_LABEL = "<label>";
static int labelTag = 0;
static S_table LOCAL_TABLE;

string count2string(int count) {
    string s = checked_malloc(DEFAULT_COUNT_SIZE);
    memset(s, 0, DEFAULT_COUNT_SIZE);
    int offset = 0;
    while (count) {
        *(s + offset) = (char)((count % 10) + 48);
        count /= 10;
    }
    return s;
}

string Label_NewFun() {
    labelTag = 0;
    LOCAL_TABLE = S_NewTable();
    return BBNAME;
}

string Label_NewLabel(string prefix) {
    if (!prefix) {
        prefix = DEFAULT_LABEL;
    }
    string tag = count2string(labelTag++);
    int prefixlen = strlen(prefix);
    int taglen = strlen(tag);
    int size = prefixlen + taglen + 1;
    string out = checked_malloc(size);
    memcpy(out, prefix, prefixlen);
    memcpy(out + prefixlen, tag, taglen);
    free(tag);
    return out;
}

void Label_NewDec(S_symbol name, void * value) {
    S_Enter(LOCAL_TABLE, name, value);
}

void * Label_FindDec(S_symbol name) {
    return S_Find(LOCAL_TABLE, name);
}

void Label_NewScope() {
    S_BeginScope(LOCAL_TABLE);
    
}

void Label_EndScope() {
    S_EndScope(LOCAL_TABLE);
}