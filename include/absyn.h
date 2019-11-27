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
            A_stmList body;
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
    enum { A_CONST, A_CALL } kind;
    A_line linno;
    union {
        struct {
            enum { A_INT, A_CHAR, A_FLOAT } kind;
            union {
                char cnum;
                int inum;
                float fnum;
            } u;
        } cons;
        struct {
            S_symbol name;
            A_expList para;
        } call;
    } u;
};

struct A_stm_ {
    enum {A_ASSIGN_STM, A_DEC_STM, A_IF_STM, A_WHILE_STM, 
    A_BREAK_STM, A_CONTINUE_STM, A_RETURN_STM } kind;
    A_line linno;
    union {
        struct {
            string symbol;
            A_exp exp;
        } assign;
        A_tyDec dec;
        struct {
            A_exp test;
            A_stmList iff;
            A_stmList elsee;
        } iff;
        struct {
            A_exp test;
            A_stmList whilee;
        } whilee;
        A_exp returnn;
    } u;
};

A_globalDec A_Fun(A_line, S_symbol, S_symbol, A_tyDecList, A_stmList);

A_globalDec A_Struct(A_line, S_symbol, A_tyDecList);

A_tyDec A_Var(A_line, S_symbol, S_symbol);

A_stm A_AssignStm(A_line, string, A_exp);

A_stm A_DecStm(A_line, A_tyDec);

A_stm A_IfStm(A_line, A_exp, A_stmList, A_stmList);

A_stm A_WhileStm(A_line, A_exp, A_stmList);

A_stm A_BreakStm(A_line);

A_stm A_ContinueStm(A_line);

A_stm A_ReturnStm(A_line, A_exp);

A_exp A_Call(A_line, S_symbol, A_expList);

A_exp A_Char(A_line, char);

A_exp A_Int(A_line, int);

A_exp A_Float(A_line, float);