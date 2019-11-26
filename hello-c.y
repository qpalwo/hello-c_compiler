%{
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#define NULL 0

extern int yylineno;

int yydebug = 1;
A_globalDecList absyn_root;

int yylex(void);

void yyerror(char * s) {

}
%}
%union {
    int ival;
    string sval;
    float fval;
    char cval;
    A_globalDec dec;
    A_globalDecList declist;
    A_expList explist;
    A_exp exp;
    A_stm stm;
    A_stmList stmlist;
    A_tyDec tydec;
    A_tyDecList tydeclist;
}

%token <sval> ID
%token <ival> INT
%token <fval> FLOAT
%token <cval> CHAR

%token
WHILE BREAK CONTINUE IF ELSE RETURN STRUCT
COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE DOT 
PLUS MINUS TIMES DIVIDE NOT AND OR ASSIGN LT GT
EQ NEQ LE GE 

%type <exp> exp assignexp callexp
%type <explist> explist
%type <dec> dec
%type <declist> declist
%type <stm> stm
%type <stmlist> stmlist
%type <tydec> tydec
%type <tydeclist> tydeclist


%start prog

%%

prog: declist { absyn_root = $1; }
;

declist:    dec { $$ = A_GlobalDecList($1, NULL); }
        |   dec declist { $$ = A_GlobalDecList($1, $2); }
;

dec:    ID ID LPAREN tydeclist RPAREN LBRACE stmlist RBRACE { $$ = A_Fun(yylineno, S_Symbol($1), S_Symbol($2), $4, $7); }
    |   STRUCT ID LBRACE tydeclist RBRACE { $$ = A_Struct(yylineno, S_Symbol($2), $4); }
;

tydeclist:  tydec { $$ = A_TyDecList($1, NULL); }
        |   tydec COMMA tydeclist { $$ = A_TyDecList($1, $3); }
;

tydec:  ID ID { $$ = A_Var(yylineno, S_Symbol($1), S_Symbol($2)); }
;

stmlist:    stm SEMICOLON { $$ = A_StmList($1, NULL); }
        |   stm SEMICOLON stmlist { $$ = A_StmList($1, $3); }
;

stm:    
;

explist:    exp { $$ = A_ExpList($1, NULL); }
        |   exp COMMA explist { $$ = A_ExpList($1, $3); }
;

exp:    assignexp { $$ = $1; }
    |   callexp { $$ = $1; }
;

assignexp:  ID ASSIGN exp { $$ = A_Assign(yylineno, $1, $3); }
        |   ID ID ASSIGN exp { $$ = A_Assign(yylineno, $2, $4); }
;

callexp:    ID LPAREN explist RPAREN {}//{ $$ = A_Call(yylineno, $1, $3); }
;

callpara: 