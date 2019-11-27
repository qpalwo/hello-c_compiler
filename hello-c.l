%{
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "parser.h"
#define YY_USER_ACTION fwrite( yytext, yyleng, 1, yyout );

int commentNest = 0;
bool hasChar = FALSE;

int yywrap() {
    return 1;
}

%}
%option yylineno

%x COMMENT_STATE CHAR_STATE
%%
" "	 {continue;}
[ \r\t] {continue;}
\n	 {continue;}

while   {return WHILE;}
if  {return IF;}
else    {return ELSE;}
struct  {return STRUCT;}
continue    {return CONTINUE;}
break   {return BREAK;}
return  {return RETURN;}

,   {return COMMA;}
"." {return DOT;}
":" {return COLON;}
";" {return SEMICOLON;}
"(" {return LPAREN;}
")" {return RPAREN;}
"[" {return LBRACK;}
"]" {return RBRACK;}
"{" {return LBRACE;}
"}" {return RBRACE;}
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
    return INT;
}

[0-9]+.[0-9]+ {
    yylval.fval = atof(yytext);
    return FLOAT;
}

"/*" {
    commentNest += 1;
    BEGIN(COMMENT_STATE);
}

'.' {
    yylval.cval = yytext[1];
    return CHAR;
}


<COMMENT_STATE>{
    "/*" {
        commentNest += 1;
        continue;
    }
    "*/" {
        commentNest -= 1;
        if (commentNest == 0) {
            BEGIN(INITIAL);
        }
    }
    <<EOF>> {
        if (commentNest > 0) {
            yyterminate();
        }
    }
    . {continue;}
}

. {
    yylval.cval = yytext[0];
    return CHAR;
}
%%