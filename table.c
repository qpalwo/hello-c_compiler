#include <assert.h>
#include <stdio.h>
#include "util.h"
#include "table.h"
#define TABLE_SIZE 127

typedef struct node_ * node;

struct node_ {
    void * key;
    void * value;
    void * pretop;
    node next;
};

struct TB_table_ {
    void * top;
    bool print;
    int curLevel;
    node table[TABLE_SIZE];
};

static node Node(void * key, void * value, void * pretop, node next) {
    node nd = checked_malloc(sizeof(*nd));
    nd->key = key;
    nd->value = value;
    nd->pretop = pretop;
    nd->next = next;
    return nd;
}


void TB_Enter(TB_table table, void * key, void * value) {
    assert(table && key);
    int index = ((unsigned) key) % TABLE_SIZE;
    table->table[index] = Node(key, value, table->top, table->table[index]);
    table->top = key;
}

void * TB_Pop(TB_table table) {
    assert(table);
    void * t = table->top;
    assert(t);
    int index = ((unsigned) t) % TABLE_SIZE;
    node n = table->table[index];
    assert(n);
    table->table[index] = n->next;
    table->top = n->pretop;
    return n->key;
}

void * TB_Find(TB_table table, void * key) {
    assert(table && key);
    int index = ((unsigned) key) % TABLE_SIZE;
    for (node n = table->table[index]; n && n->key; n = n->next) {
        if (key == n->key)
            return n->value;
    }
    return NULL;
}

TB_table TB_New(bool print) {
    TB_table table = checked_malloc(sizeof(*table));
    table->top = NULL;
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->table[i] = NULL;
    }
    table->curLevel = 0;
    table->print = print;
    return table;
}

void TB_AddLevel(TB_table table) {
    table->curLevel++;
}

void TB_DownLevel(TB_table table) {
    table->curLevel--;
    assert(table->curLevel >= 0);
}

bool TB_GetDebug(TB_table table) {
    return table->print;
}

int TB_GetLevel(TB_table table) {
    return table->curLevel;
}