%{
    #include <bits/stdc++.h>
    #include "y.tab.hpp"
    extern char mytext[10000];
%}

letter [A-Za-z]
digit [0-9]
underscore [_]
keywords [A-Za-z0-9_]
dollar [$]

%%

"int"			                            {return INT;}
"string"                                    {return STRING;}
"character"                                 {return CHARACTER;}

"for"                                       {return FOR;}
"while"                                     {return WHILE;}

"if"                                        {return IF;}
"else"                                      {return ELSE;}

"print"                                     {return PRINT;}
"scan"                                      {return SCAN;}

"array"                                     {return ARRAY;}

"return"                                    {return RETURN;}

"=="                                        {return EE;}
"!="                                        {return NE;}
"<="                                        {return LE;}
"<"                                         {return LT;}
">="                                        {return GE;}
">"                                         {return GT;}     

"||"                                        {return OR;}
"&&"                                        {return AND;}
"!"                                         {return NOT;}

"|"                                         {return BOR;}
"&"                                         {return BAND;}
"^"                                         {return BXOR;}

","										    {return COMMA;}
";"										    {return SEMICOLON;}


"{"										    {return LCB;}
"}"										    {return RCB;}
"("                                         {return LNB;}
")"                                         {return RNB;}
"["                                         {return LSB;}
"]"                                         {return RSB;}

"++"                                        {return INC;}
"--"                                        {return DEC;}

"+"                                         {return PLUS;}
"-"                                         {return MINUS;}
"*"                                         {return MULTIPLY;}
"/"                                         {return DIVIDE;}
"="                                         {return EQUALTO;}


({letter}{keywords}*)                               {strcpy(mytext, yytext);return IDENTIFIER;}

(["]({letter}|{digit}|[" "]|{underscore})*["])      {strcpy(mytext, yytext);return STRING_CONSTANT;}
([']({letter}|{digit}|[" "]|{underscore})['])       {strcpy(mytext, yytext);return CHARACTER_CONSTANT;}

{dollar}{letter}{keywords}*                        {strcpy(mytext, yytext); return FUNCTION_IDENTIFIER;}

([-]{digit}+)				                        {strcpy(mytext, yytext);return NEG_INT;}
({digit}+)				                            {strcpy(mytext, yytext);return POS_INT;}

[ \t\r\n]	                                        ;
.                                                   ;
"#".*                                              {} 

%%

int yywrap(void){
    return 1;
}