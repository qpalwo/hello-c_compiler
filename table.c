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