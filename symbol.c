#include <stdio.h>
#include <string.h>
#include "util.h"
#include "symbol.h"


struct S_symbol_ {string name; S_symbol next;};

static S_symbol mksymbol(string name, S_symbol next)
{S_symbol s=checked_malloc(sizeof(*s));
 s->name=name; s->next=next;
 return s;
}

#define SIZE 109  /* should be prime */

static S_symbol hashtable[SIZE];

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

S_symbol S_Symbol(string name) {
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
 
string S_name(S_symbol sym) {
    return sym->name;
}