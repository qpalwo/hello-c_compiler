typedef char bool;
typedef char *string;

#define TRUE 1
#define FALSE 0
// #define NULL 0

void *checked_malloc(int);
string String(char *);
void ErrorMsg(int pos, char *message,...);