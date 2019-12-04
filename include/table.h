
/* 仅做语义分析，中间代码符号表交给 LLVM 管理 */
typedef struct TB_table_ * TB_table;

void TB_Enter(TB_table, void * key, void * value);

void * TB_Pop(TB_table);

void * TB_Find(TB_table, void * key);

void TB_AddLevel(TB_table table);

void TB_DownLevel(TB_table table);

bool TB_GetDebug(TB_table table);

int TB_GetLevel(TB_table table);

TB_table TB_New(bool);
