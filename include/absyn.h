#define LIST_STRUCT(name) \
typedef struct A_##name##List_ * A_##name##List;\
struct A_##name##List_ { \
    A_##name head;\
    A_##name##List tail;\
};

#define LIST_FUN(name, Name) \
A_##name##List A_##Name##List(A_##name head, A_##name##List tail) { \
    A_##name##List list = checked_malloc(sizeof(*list)); \
    list->head = head; \
    list->tail = tail; \
    return tail; \
}

#define LIST_DEFINE(name, Name) \
TYPE_DEF(name) \
LIST_STRUCT(name) \
A_##name##List A_##Name##List(A_##name, A_##name##List);

#define TYPE_DEF(name) \
typedef struct A_##name##_ * A_##name;

typedef int A_line;

LIST_DEFINE(stm, Stm)
LIST_DEFINE(exp, Exp)
LIST_DEFINE(globalDec, GlobalDec)
LIST_DEFINE(tyDec, TyDec)

struct A_globalDec_ {
    enum { A_FUN, A_STRUCT } kind;
    A_line linno;
    union {
        struct {
            A_line linno;
            S_symbol ret;
            S_symbol name;
            A_tyDecList para;
            A_expList body;
        } fun;
        struct {
            S_symbol name;
            A_tyDecList declist;
        } struc;
    } u;
};

struct A_tyDec_ {
    enum { A_VAR, A_ARRAY } kind;
    A_line linno;
    union {
        struct {
            S_symbol type;
            S_symbol name;
        } var;
    } u;
};

struct A_exp_ {
    enum {A_CALL} kind;
    A_line linno;
    union {
        struct {
            string name;
            A_expList para;
        } call;
    } u;
};

struct A_stm_ {
    enum {A_ASSIGN} kind;
    A_line linno;
    union {
        struct {
            string symbol;
            A_exp exp;
        } assign;
    } u;
};

A_globalDec A_Fun(A_line, S_symbol, S_symbol, A_tyDecList, A_expList);

A_globalDec A_Struct(A_line, S_symbol, A_tyDecList);

A_tyDec A_Var(A_line, S_symbol, S_symbol);

A_stm A_Assign(A_line, string, A_exp);

A_exp A_Call(A_line, string, A_expList);