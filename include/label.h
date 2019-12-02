typedef struct S_symbol_ * S_symbol;
string count2string(int count);

string Label_NewFun();

string Label_NewLabel(string prefix);

void Label_NewDec(S_symbol name, void * value);

void * Label_FindDec(S_symbol name);

void Label_NewScope();

void Label_EndScope();