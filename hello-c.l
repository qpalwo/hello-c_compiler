%{
#include "util.h"
#include "absyn.h"
#include "parser.h"
#define YY_USER_ACTION fwrite( yytext, yyleng, 1, yyout );

int yywrap() {
    return 1;
}

%}
%option yylineno

%%
" "	 {continue;}
[ \r\t] {continue;}
\n	 {continue;}

while   {return WHILE;}
if  {return IF;}
else    {return ELSE;}
void    {return VOID;}
int {return INT;}
float   {return FLOAT;}
continue    {return CONTINUE;}
break   {return BREAK;}
return  {return RETURN;}

,   {return COMMA;}
":" {return COLON;}
";" {return SEMICOLON;}
"(" {return LPAREN;}
")" {return RPAREN;}
"[" {return LBRACK;}
"]" {return RBRACK;}
"{" {return LBRACE;}
"}" {return RBRACE;}
"." {return DOT;}
"+" {return PLUS;}
"-" {return MINUS;}
"*" {return TIMES;}
"/" {return DIVIDE;}
"=" {return ASSIGN;}
"!" {return NOT;}
"&" {return AND;}
"|" {return OR;}
"<" {return LT;}
">" {return GT;}
"<=" {return LE;}
">=" {return GE;}
"==" {return EQ;}
"!=" {return NEQ;}

[a-z|A-Z]+[a-z|A-Z|0-9|_]* {
    yylval.sval = yytext;
    return ID;
}

[0-9]+ {
    yylval.ival = atoi(yytext); 
    return INUM;
}

[0-9]+.[0-9]+ {
    yylval.fval = atof(yytext);
    return FNUM;
}
%%