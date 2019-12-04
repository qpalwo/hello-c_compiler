#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "type.h"
#include "table.h"
#include "symbol.h"

extern int TABLE_DEBUG;

struct S_symbol_ {
    string name; 
    S_symbol next;
};

static S_symbol mksymbol(string name, S_symbol next) {
    S_symbol s = checked_malloc(sizeof(*s));
    // strcpy(s->name, name);
    s->name = name; 
    s->next = next;
    return s;
}

#define SIZE 109  /* should be prime */

static S_symbol hashtable[SIZE];
static struct S_symbol_ mask = { "<mask>", NULL };

static unsigned int hash(char *s0) {
    unsigned int h=0; 
    char *s;
    for(s = s0; *s; s++) {
        h = h * 65599 + *s;
    }
    return h;
}
 
static int streq(string a, string b) {
    return !strcmp(a,b);
}

static void Indent(int tabs) {
    for (int i = 0; i < tabs; i++) {
        printf("\t");
    }
}

S_symbol S_Symbol(const string name) {
    int index= hash(name) % SIZE;
    S_symbol syms = hashtable[index], sym;
    for(sym=syms; sym; sym=sym->next) {
        if (streq(sym->name,name)) {
            return sym;
        }
    }
    sym = mksymbol(name,syms);
    hashtable[index]=sym;
    return sym;
}
 
string S_Name(S_symbol sym) {
    assert(sym);
    return sym->name;
}

void S_Enter(S_table table, S_symbol key, void * value) {
    TB_Enter(table, key, value);
    if (TB_GetDebug(table) && TABLE_DEBUG && key != &mask) {
        Indent(TB_GetLevel(table));
        printf("<enter> %s\n", S_Name(key));
    }
}

void * S_Pop(S_table table) {
    return TB_Pop(table);
}

void * S_Find(S_table table, S_symbol key) {
    return TB_Find(table, key);
}

S_table S_NewTable(bool print) {
    return TB_New(print);
}

void S_BeginScope(S_table table) {
    S_Enter(table, &mask, NULL);
    TB_AddLevel(table);
    if (TB_GetDebug(table) && TABLE_DEBUG) {
        printf("--------- start scope level %d --------------\n", TB_GetLevel(table));
    }
}

void S_EndScope(S_table table) {
    if (TB_GetDebug(table) && TABLE_DEBUG) {
        printf("----------- end scope level %d --------------\n", TB_GetLevel(table));
    }
    TB_DownLevel(table);
    while (S_Pop(table) != &mask);
}