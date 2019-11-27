
void printGlobalDec(FILE * out, A_globalDec dec, int deepth);
void printAST(FILE* out, A_globalDecList tree);
void printStm(FILE * out, A_stm stm, int deepth);
void printStmList(FILE * out, A_stmList root, int deepth);
void printTypeDec(FILE * out, A_tyDec dec, int deepth);
void printTypeDecList(FILE * out, A_tyDecList root, int deepth);
void printVar(FILE * out, A_var var, int deepth);
void printExp(FILE * out, A_exp exp, int deepth);
void printExpList(FILE * out, A_expList root, int deepth);
