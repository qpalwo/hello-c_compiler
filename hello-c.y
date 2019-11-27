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
    A_var var;
    A_exp exp;
    A_expList explist;
    A_stm stm;
    A_stmList stmlist;
    A_tyDec tydec;
    A_tyDecList tydeclist;
}

%right ASSIGN
%nonassoc LT GT LE GE EQ NEQ 
%left PLUS MINUS
%left TIMES DIVIDE
%left SPLUS SMINUS
%left BITAND BITOR

%token <sval> ID
%token <ival> INT
%token <fval> FLOAT
%token <cval> CHAR

%token
WHILE BREAK CONTINUE IF ELSE RETURN STRUCT
COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE DOT 
// PLUS MINUS TIMES DIVIDE NOT AND OR ASSIGN LT GT
NOT AND OR SPLUS SMINUS
// EQ NEQ LE GE 


%type <exp> exp arrayexp callexp constexp singexp doublexp
%type <explist> explist arrayexplist
%type <dec> dec
%type <declist> declist
%type <var> var
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

stm:    tydec SEMICOLON { $$ = A_DecStm(yylineno, $1); }
    |   assignstm SEMICOLON { $$ = $1; }
    |   whilestm { $$ = $1; }
    |   ifstm { $$ = $1; }
    |   BREAK SEMICOLON { $$ = A_BreakStm(yylineno);}
    |   CONTINUE SEMICOLON { $$ = A_ContinueStm(yylineno); }
    |   RETURN exp SEMICOLON { $$ = A_ReturnStm(yylineno, $2); }
;

exp:    constexp { $$ = $1; }
    |   singexp { $$ = $1; }
    |   doublexp { $$ = $1; }
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
        |   var { $$ = A_VarExp(yylineno, $1); }
;

singexp:    exp SPLUS { $$ = A_SingleExp(yylineno, A_SPLUS, $1); }
        |   exp SMINUS { $$ = A_SingleExp(yylineno, A_SMINUS, $1); }
        |   NOT exp { $$ = A_SingleExp(yylineno, A_NOT, $2); }
        |   MINUS exp { $$ = A_SingleExp(yylineno, A_NEGATIVE, $2); }
        |   PLUS exp { $$ = A_SingleExp(yylineno, A_POSITIVE, $2); }
;

doublexp:   exp PLUS exp { $$ = A_DoubleExp(yylineno, A_PLUS, $1, $3); }
        |   exp MINUS exp { $$ = A_DoubleExp(yylineno, A_MINUS, $1, $3); }
        |   exp TIMES exp { $$ = A_DoubleExp(yylineno, A_TIMES, $1, $3); }
        |   exp DIVIDE exp { $$ = A_DoubleExp(yylineno, A_DIVIDE, $1, $3); }
        |   exp EQ exp { $$ = A_DoubleExp(yylineno, A_EQ, $1, $3); }
        |   exp NEQ exp { $$ = A_DoubleExp(yylineno, A_NEQ, $1, $3); }
        |   exp GT exp { $$ = A_DoubleExp(yylineno, A_GT, $1, $3); }
        |   exp GE exp { $$ = A_DoubleExp(yylineno, A_GE, $1, $3); }
        |   exp LT exp { $$ = A_DoubleExp(yylineno, A_LT, $1, $3); }
        |   exp LE exp { $$ = A_DoubleExp(yylineno, A_LE, $1, $3); }
        |   exp AND exp { $$ = A_DoubleExp(yylineno, A_AND, $1, $3); }
        |   exp OR exp { $$ = A_DoubleExp(yylineno, A_OR, $1, $3); }
        |   exp BITAND exp { $$ = A_DoubleExp(yylineno, A_BITAND, $1, $3); }
        |   exp BITOR exp { $$ = A_DoubleExp(yylineno, A_BITOR, $1, $3); }
;

assignstm:  var ASSIGN exp { $$ = A_AssignStm(yylineno, $1, $3); }
;

whilestm:   WHILE LPAREN exp RPAREN LBRACE stmlist RBRACE { $$ = A_WhileStm(yylineno, $3, $6); }
;

arrayexp:   LBRACK exp RBRACK { $$ = $2; }
;

arrayexplist:   arrayexp { $$ = A_ExpList($1, NULL); }
            |   arrayexp arrayexplist { $$ = A_ExpList($1, $2); }
;

var:    ID { $$ = A_SymbolVar(yylineno, S_Symbol($1)); }
    |   ID DOT ID { $$ = A_StructVar(yylineno, S_Symbol($1), S_Symbol($3)); }
    |   ID arrayexplist { $$ = A_ArrayVar(yylineno, S_Symbol($1), $2); }
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