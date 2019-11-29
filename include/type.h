typedef struct TY_entry_ * TY_entry;
typedef struct TY_entryList_ * TY_entryList;

struct TY_entryList_ {
    TY_entry head;
    TY_entryList tail;
};


struct TY_entry_ {
    enum {
        TY_VOID,
        TY_INT,
        TY_FLOAT,
        TY_CHAR,
        TY_ARRAY,
        TY_STRUCT,
        TY_FUN
    } kind;
    union {
        struct {
            TY_entry type;
            int level;
        } array;
        TY_entryList struc;
        struct {
            TY_entry ret;
            TY_entryList para;
        } fun;
    } u;
};

TY_entry TY_Void();

TY_entry TY_Int();

TY_entry TY_Float();

TY_entry TY_Char();

TY_entry TY_Array(TY_entry type, int level);

TY_entry TY_Struct(TY_entryList types);

TY_entry TY_Fun(TY_entry ret, TY_entryList paras);

TY_entryList TY_EntryList(TY_entry head, TY_entryList tail);