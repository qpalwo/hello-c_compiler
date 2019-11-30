typedef struct S_symbol_ * S_symbol;
typedef struct TB_table_ * S_table;

/* Make a unique symbol from a given string.  
 *  Different calls to S_Symbol("foo") will yield the same S_symbol
 *  value, even if the "foo" strings are at different locations. */
S_symbol S_Symbol(const string);

/* Extract the underlying string from a symbol */
string S_Name(S_symbol);

void S_Enter(S_table, S_symbol, void *);

void * S_Pop(S_table);

void * S_Find(S_table, S_symbol);

void S_BeginScope(S_table);

void S_EndScope(S_table);

S_table S_NewTable();