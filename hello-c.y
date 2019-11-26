%{
#include "util.h"
#include "absyn.h"

extern int yylineno;

int yydebug = 1;
A_expList absyn_root;

int yylex(void);

void yyerror(char * s) {

}
%}
%union {
    int ival;
    string sval;
    float fval;
    A_expList explist;
    A_exp exp;
    A_type type;
}

%token <sval> ID
%token <ival> INUM
%token <fval> FNUM

%token
WHILE VOID INT FLOAT BREAK CONTINUE IF ELSE
COMMA SEMICOLON LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE DOT PLUS MINUS TIMES DIVIDE AND OR

%type <exp> exp funexp
%type <para> para
%type <explist> explist
%type <paralist> paralist
%type <type> type


%start prog

%%

prog: explist { absyn_root = $1; }
;

exp:
;

explist:    exp SEMICOLON { $$ = A_ExpList($1, NULL); }
        |   exp SEMICOLON explist { $$ = A_ExpList($1, $3); }
;


para:   type ID { $$ = A_Field(yylineno, $1, $2); }
;

paralist: LPAREN para COMMA paralist RPAREN {$$ = A_FieldList($2, $4); }
        |   LPAREN para RPAREN { $$ = A_FieldList($2, NULL); }
        |   para COMMA paralist { $$ = A_FieldList($1, $3); }
        |   para { $$ = A_FieldList($1, NULL); }
;

funexp: type ID paralist LBRACE explist RBRACE { $$ = A_Fun(yylineno, $1, $2, $3, $5); }
;

type:     VOID { $$ = A_Void(yylineno); }
            |   INT { $$ = A_Int(yylineno); }
            |   FLOAT { $$ = A_Float(yylineno); }
;