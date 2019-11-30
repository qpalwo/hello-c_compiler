#include <stdio.h>
#include <string.h>
#include "util.h"
#include "type.h"
#include "table.h"
#include "symbol.h"

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
    return sym->name;
}

void S_Enter(S_table table, S_symbol key, void * value) {
    TB_Enter(table, key, value);
}

void * S_Pop(S_table table) {
    return TB_Pop(table);
}

void * S_Find(S_table table, S_symbol key) {
    return TB_Find(table, key);
}

S_table S_NewTable() {
    return TB_New();
}

void S_BeginScope(S_table table) {
    S_Enter(table, &mask, NULL);
}

void S_EndScope(S_table table) {
    while (S_Pop(table) != &mask);
}