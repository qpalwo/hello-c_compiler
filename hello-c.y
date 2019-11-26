%{
#include "util.h"
#include "absyn.h"
#define NULL 0

extern int yylineno;

int yydebug = 1;
A_funList absyn_root;

int yylex(void);

void yyerror(char * s) {

}
%}
%union {
    int ival;
    string sval;
    float fval;
    A_fun fundec;
    A_funList funlist;
    A_expList explist;
    A_exp exp;
    A_stm stm;
    A_stmList stmlist;
    A_type type;
    A_field para;
    A_fieldList paralist;
}

%token <sval> ID
%token <ival> INUM
%token <fval> FNUM

%token
WHILE VOID INT FLOAT BREAK CONTINUE IF ELSE RETURN
COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE DOT 
PLUS MINUS TIMES DIVIDE NOT AND OR ASSIGN LT GT
EQ NEQ LE GE 

%type <exp> exp assignexp callexp
%type <para> para
%type <explist> explist
%type <paralist> paralist
%type <fundec> fundec
%type <funlist> funlist
%type <type> type
%type <stm> stm
%type <stmlist> stmlist


%start prog

%%

prog: funlist { absyn_root = $1; }
;

funlist:    fundec { $$ = A_FunList($1, NULL); }
        |   fundec funlist { $$ = A_FunList($1, $2); }
;

fundec: type ID LPAREN paralist RPAREN LBRACE stmlist RBRACE { $$ = A_Fun(yylineno, $1, $2, $4, $7); }
;

paralist:   para { $$ = A_FieldList($1, NULL); }
        |   para COMMA paralist { $$ = A_FieldList($1, $3); }
;

para:   type ID { $$ = A_Field(yylineno, $1, $2); }
;

type:   VOID { $$ = A_Void(yylineno); }
    |   INT { $$ = A_Int(yylineno); }
    |   FLOAT { $$ = A_Float(yylineno); }
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
        |   type ID ASSIGN exp { $$ = A_Assign(yylineno, $2, $4); }
;

callexp:    ID LPAREN explist RPAREN {}//{ $$ = A_Call(yylineno, $1, $3); }
;

callpara: 