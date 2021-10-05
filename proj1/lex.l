%{
    #include "syntax.tab.h"
    #include "stdlib.h"
%}

unic u[0-9a-fA-F]{4}
esc \\([\"\\/bfnrt]|{unic})
scp [^"\\\x00-\x1f]
string \"({esc}|{scp})*\"

digit         [0-9]
hex-digit     [0-9a-fA-F]
letter        [A-Za-z]

dec 0|[1-9]{digit}*
hex 0[xX](0|[1-9a-fA-F]{hex-digit}*)
integer -?({dec}|{hex})
frac \.{digit}+
float -?{dec}{frac}
exp [Ee][+-]?[0-9]+

empty [ \n\r\t]

%%

"{" { yylval = init_node("LC", NULL, 0); return LC; }
"}" { yylval = init_node("RC", NULL, 0); return RC; }
"[" { yylval = init_node("LB", NULL, 0); return LB; }
"]" { yylval = init_node("RB", NULL, 0); return RB; }
"(" { yylval = init_node("LP", NULL, 0); return LP; }
")" { yylval = init_node("RP", NULL, 0); return RP; }

"+" { yylval = init_node("ADD", NULL, 0); return ADD; }
"-" { yylval = init_node("SUB", NULL, 0); return SUB; }
"*" { yylval = init_node("MUL", NULL, 0); return MUL; }
"/" { yylval = init_node("DIV", NULL, 0); return DIV; }
"&&" { yylval = init_node("AND", NULL, 0); return AND; }
"||" { yylval = init_node("OR", NULL, 0); return OR; }
"!"  { yylval = init_node("NOT", NULL, 0); return NOT; }
"="  { yylval = init_node("ASSIGN", NULL, 0); return ASSIGN; }
"<"  { yylval = init_node("LT", NULL, 0); return LT; }
"<=" { yylval = init_node("LE", NULL, 0); return LE; }
">"  { yylval = init_node("GT", NULL, 0); return GT; }
">=" { yylval = init_node("GE", NULL, 0); return GE; }
"!=" { yylval = init_node("NE", NULL, 0); return NE; }
"==" { yylval = init_node("EQ", NULL, 0); return EQ; }


"," { yylval = init_node("COMMA", NULL, 0); return COMMA; }

"true" { yylval = init_node("TRUE", NULL, 0); return TRUE; }
"false" { yylval = init_node("FALSE", NULL, 0); return FALSE; }
"null" { yylval = init_node("VNULL", NULL, 0); return VNULL; }

{string} { yylval = init_node("STRING", NULL, 0); return STRING; }
{empty} {}

. { printf("lexical error: %s\n", yytext); }

%%