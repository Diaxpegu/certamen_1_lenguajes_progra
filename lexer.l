%option noinput nounput

%{
#include "parser.tab.h"
#include <string.h>
%}

%%
"maquina"       { return TOKEN_MAQUINA; }
"transiciones"  { return TOKEN_TRANSICIONES; }
"DER"           { return TOKEN_DER; }
"IZQ"           { return TOKEN_IZQ; }
"->"            { return TOKEN_FLECHA; }
"alfabeto"      { return TOKEN_ALFABETO; }
"estados"       { return TOKEN_ESTADOS; }
"inicial"       { return TOKEN_INICIAL; }
"finales"       { return TOKEN_FINALES; }
"QUIETO"        { return TOKEN_QUIETO; }
":"             { return ':'; }
"subrutina"     { return TOKEN_SUBRUTINA; }
"usa"           {return TOKEN_USA;}
[0-9]+          { yylval.num = atoi(yytext); return TOKEN_NUMERO; }
[a-zA-Z0-9_]+   { yylval.str = strdup(yytext); return TOKEN_ID; }
"//".*          { ; }
[ \t\n\r]+      { ; }
.               { return yytext[0]; }
%%              

int yywrap() { return 1; }

                
              
