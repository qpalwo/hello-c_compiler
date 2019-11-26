%{
#include "util.h"
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

,   {return COMMA;}
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
"&" {return AND;}
"|" {return OR;}

[a-z|A-Z]+[a-z|A-Z|0-9|_]* {
    yylval.id = yytext;
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