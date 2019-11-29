
/* 仅做语义分析，中间代码符号表交给 LLVM 管理 */
typedef struct TB_table_ * TB_table;

void TB_Enter(TB_table, void * key, void * value);

void * TB_Pop(TB_table);

void * TB_Find(TB_table, void * key);

TB_table TB_New();
