
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
LIST_STRUCT(name) \
A_##name##List A_##Name##List(A_##name, A_##name##List);

typedef int A_line;
typedef struct A_type_ * A_type;
typedef struct A_data_ * A_data;
LIST_DEFINE(data, Data)
typedef struct A_fun_ * A_fun;
LIST_DEFINE(fun, Fun)
typedef struct A_stm_ * A_stm;
LIST_DEFINE(stm, Stm)
typedef struct A_exp_ * A_exp;
LIST_DEFINE(exp, Exp)
typedef struct A_type_ * A_type;
typedef struct A_field_ * A_field;
LIST_DEFINE(field, Field)

struct A_type_ {
    enum {A_VOID, A_INT, A_FLOAT, A_SYMBOL} kind;
    A_line linno;
};

struct A_data_ {
    A_type type;
    union {
        int ival;
        float fval;
        string symbol;
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

struct A_fun_ {
    A_line linno;
    A_type ret;
    string name;
    A_fieldList para;
    A_expList body;
};

struct A_field_ {
    A_line linno;
    A_type type;
    string name;
};

A_fun A_Fun(A_line, A_type, string, A_fieldList, A_expList);

A_type A_Void(A_line);

A_type A_Int(A_line);

A_type A_Float(A_line);

A_type A_Symbol(A_line);

A_data A_IData(A_line, int);

A_data A_FData(A_line, float);

A_data A_VData(A_line);

A_data A_SData(A_line, string);

A_field A_Field(A_line, A_type, string);

A_stm A_Assign(A_line, string, A_exp);

A_exp A_Call(A_line, string, A_expList);