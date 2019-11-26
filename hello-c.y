%{
#include "util.h"
extern int yylineno;

int yydebug = 1;
int yylex(void);

void yyerror(char * s) {

}
%}
%union {
    int ival;
    string id;
    float fval;
}

%token
WHILE VOID INT FLOAT BREAK CONTINUE IF ELSE
COMMA SEMICOLON LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE DOT PLUS MINUS TIMES DIVIDE AND OR
ID INUM FNUM

%start prog

%%

prog: VOID {;}
;

// typedef: 
// ;

// paralist: LPAREN paralist RPAREN
// ;

// fundef: returntype ID paralist funbody {;}
// ;

// returntype:     VOID {;}
//             |INT {;}
//             |FLOAT {;}
// ;