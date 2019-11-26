

typedef int A_line;
typedef struct A_data_ * A_data;
typedef struct A_fun_ * A_fun;


struct A_data_ {
    enum {A_VOID, A_INT, A_FLOAT} kind;
    A_line linno;
    union {
        int ival;
        float fval;
    } u;
};

struct A_fun_ {
    A_line linno;
    A_data ret;

};