/* 
 * Copyright 2019 Yuxuan Xiao

 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
%{
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "parser.h"
#define YY_USER_ACTION \
{ \
    if (lexPrint) { \
        fwrite( yytext, yyleng, 1, yyout ); \
    } \
} 

int lexPrint = 0;
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
"&" {return BITAND;}
"|" {return BITOR;}
"<" {return LT;}
">" {return GT;}
"<=" {return LE;}
">=" {return GE;}
"==" {return EQ;}
"!=" {return NEQ;}
"++" {return SPLUS;}
"--" {return SMINUS;}
"||" {return OR;}
"&&" {return AND;}

[a-z|A-Z]+[a-z|A-Z|0-9|_]* {
    yylval.sval = strdup(yytext);
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