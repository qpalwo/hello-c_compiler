%{
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#define NULL 0

extern int yylineno;

int yydebug = 0;
A_globalDecList absyn_root;

int yylex(void);

void yyerror(char * s) {
    ErrorMsg(yylineno, "%s", s);
}
%}

%union {
    int ival;
    string sval;
    float fval;
    char cval;
    A_globalDec dec;
    A_globalDecList declist;
    A_exp exp;
    A_expList explist;
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

%type <exp> exp callexp constexp
%type <explist> explist
%type <dec> dec
%type <declist> declist
%type <stm> stm assignstm whilestm ifstm
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
    |   { $$ = NULL; }
;

stmlist:    stm { $$ = A_StmList($1, NULL); }
        |   stm stmlist { $$ = A_StmList($1, $2); }
;

stm:    { $$ = NULL; }
    |   tydec SEMICOLON { $$ = A_DecStm(yylineno, $1); }
    |   assignstm SEMICOLON { $$ = $1; }
    |   whilestm { $$ = $1; }
    |   ifstm { $$ = $1; }
    |   BREAK SEMICOLON { $$ = A_BreakStm(yylineno);}
    |   CONTINUE SEMICOLON { $$ = A_ContinueStm(yylineno); }
    |   RETURN exp SEMICOLON { $$ = A_ReturnStm(yylineno, $2); }
;

exp:    constexp { $$ = $1; }
    |   callexp { $$ = $1; }
;

explist:    exp { $$ = A_ExpList($1, NULL); }
        |   exp COMMA explist { $$ = A_ExpList($1, $3); }
;

callexp:    ID LPAREN explist RPAREN { $$ = A_Call(yylineno, S_Symbol($1), $3); }
;

constexp:   CHAR { $$ = A_Char(yylineno, $1); }
        |   INT { $$ = A_Int(yylineno, $1); }
        |   FLOAT { $$ = A_Float(yylineno, $1); }

assignstm:  ID ASSIGN exp { $$ = A_AssignStm(yylineno, $1, $3); }
;

whilestm:   WHILE LPAREN exp RPAREN LBRACE stmlist RBRACE { $$ = A_WhileStm(yylineno, $3, $6); }
;

ifstm:  IF LPAREN exp RPAREN LBRACE
                stmlist
                RBRACE { $$ = A_IfStm(yylineno, $3, $6, NULL); }
    |   IF LPAREN exp RPAREN LBRACE 
                stmlist 
                RBRACE ELSE LBRACE 
                stmlist RBRACE { $$ = A_IfStm(yylineno, $3, $6, $10); }
    |   IF LPAREN exp RPAREN LBRACE 
                stmlist 
                RBRACE ELSE ifstm { $$ = A_IfStm(yylineno, $3, $6, A_StmList($9, NULL)); }
;