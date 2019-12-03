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
    return list; \
}

#define LIST_DEFINE(name, Name) \
TYPE_DEF(name) \
LIST_STRUCT(name) \
A_##name##List A_##Name##List(A_##name, A_##name##List);

#define TYPE_DEF(name) \
typedef struct A_##name##_ * A_##name;

typedef int A_line;
typedef enum { A_SPLUS, A_SMINUS, A_NEGATIVE, A_POSITIVE, A_NOT } sop;
typedef enum { A_PLUS, A_MINUS, A_TIMES, A_DIVIDE, A_EQ, A_NEQ,
            A_GT, A_GE, A_LT, A_LE, A_BITAND, A_BITOR, A_AND, A_OR } dop;

TYPE_DEF(var)
LIST_DEFINE(stm, Stm)
LIST_DEFINE(exp, Exp)
LIST_DEFINE(globalDec, GlobalDec)
LIST_DEFINE(tyDec, TyDec)

struct A_globalDec_ {
    enum { A_FUN, A_STRUCT, A_GLOBAL_VAR } kind;
    A_line linno;
    union {
        struct {
            S_symbol ret;
            S_symbol name;
            A_tyDecList para;
            A_stmList body;
        } fun;
        struct {
            S_symbol name;
            A_tyDecList declist;
        } struc;
        struct {
            S_symbol name;
            S_symbol type;
        } var;
    } u;
};

struct A_tyDec_ {
    enum { A_VAR_DEC, A_ARRAY_DEC, A_NULL_DEC } kind;
    A_line linno;
    union {
        struct {
            S_symbol type;
            S_symbol name;
        } var;
        struct {
            S_symbol type;
            S_symbol name;
            A_expList exp;
        } array;
    } u;
};

struct A_var_ {
    enum { A_SYMBOL_VAR, A_ARRAY_VAR, A_STRUCT_VAR } kind;
    A_line linno;
    union {
        S_symbol symbol;
        struct {
            S_symbol symbol;
            A_expList exp;
        } arrayvar;
        struct {
            S_symbol para;
            S_symbol child;
        } structvar;
    } u;
};

struct A_exp_ {
    enum { A_CONST, A_CALL, A_DOUBLE_EXP, A_SINGLE_EXP } kind;
    A_line linno;
    union {
        struct {
            enum { A_INT, A_CHAR, A_FLOAT, A_VAR } kind;
            union {
                char cnum;
                int inum;
                float fnum;
                A_var var;
            } u;
        } cons;
        struct {
            S_symbol name;
            A_expList para;
        } call;
        struct {
            sop op;
            A_exp exp;
        } singexp;
        struct {
            dop op;
            A_exp left;
            A_exp right;
        } doublexp;
    } u;
};

struct A_stm_ {
    enum {A_ASSIGN_STM, A_DEC_STM, A_IF_STM, A_WHILE_STM, 
    A_BREAK_STM, A_CONTINUE_STM, A_RETURN_STM, A_EXP_STM } kind;
    A_line linno;
    union {
        struct {
            A_var symbol;
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
        A_exp exp;
    } u;
};

A_globalDec A_Fun(A_line, S_symbol, S_symbol, A_tyDecList, A_stmList);

A_globalDec A_Struct(A_line, S_symbol, A_tyDecList);

A_globalDec A_GlobalVar(A_line, S_symbol, S_symbol);

A_tyDec A_VarDec(A_line, S_symbol, S_symbol);

A_tyDec A_ArrayDec(A_line, S_symbol, S_symbol, A_expList);

A_tyDec A_NullDec(A_line);

A_var A_SymbolVar(A_line, S_symbol);

A_var A_ArrayVar(A_line, S_symbol, A_expList);

A_var A_StructVar(A_line, S_symbol, S_symbol);

A_stm A_AssignStm(A_line, A_var, A_exp);

A_stm A_DecStm(A_line, A_tyDec);

A_stm A_IfStm(A_line, A_exp, A_stmList, A_stmList);

A_stm A_WhileStm(A_line, A_exp, A_stmList);

A_stm A_BreakStm(A_line);

A_stm A_ContinueStm(A_line);

A_stm A_ReturnStm(A_line, A_exp);

A_stm A_ExpStm(A_line, A_exp);

A_exp A_Call(A_line, S_symbol, A_expList);

A_exp A_SingleExp(A_line, sop, A_exp);

A_exp A_DoubleExp(A_line, dop, A_exp, A_exp);

A_exp A_Char(A_line, char);

A_exp A_Int(A_line, int);

A_exp A_Float(A_line, float);

A_exp A_VarExp(A_line, A_var);