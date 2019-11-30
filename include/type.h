typedef struct TY_entry_ * TY_entry;
typedef struct TY_entryList_ * TY_entryList;
typedef struct TY_structData_ * TY_structData;
typedef struct TY_structDataList_ * TY_structDataList;
typedef struct S_symbol_ * S_symbol;

struct TY_entryList_ {
    TY_entry head;
    TY_entryList tail;
};

struct TY_structDataList_ {
    TY_structData head;
    TY_structDataList tail;
};

struct TY_structData_ {
    S_symbol name;
    TY_entry type;
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
        struct {
            TY_structDataList sdl;
            S_symbol sname;
        } struc;
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

TY_entry TY_Struct(S_symbol name, TY_structDataList types);

TY_entry TY_Fun(TY_entry ret, TY_entryList paras);

TY_entryList TY_EntryList(TY_entry head, TY_entryList tail);

TY_structData TY_StructData(S_symbol name, TY_entry type);

TY_structDataList TY_StructDataList(TY_structData head, TY_structDataList tail);