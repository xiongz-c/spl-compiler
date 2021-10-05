%{
    #include "syntax.tab.h"
    #include "stdlib.h"
    #include "ast.h"
%}
%option yylineno
unic u[0-9a-fA-F]{4}
esc \\([\"\\/bfnrt]|{unic})
scp [^"\\\x00-\x1f]
string \"({esc}|{scp})*\"

digit         [0-9]
hex-digit     [0-9a-fA-F]
letter        [A-Za-z]
_letter       [a-zA-Z_]

identifier ({letter}|_)({digit}|{letter}|_)*

dec 0|[1-9]{digit}*
hex 0[xX](0|[1-9a-fA-F]{hex-digit}*)
integer -?({dec}|{hex})
frac \.{digit}+
float -?{dec}{frac}
exp [Ee][+-]?[0-9]+
char ('{_letter}')|('\\[xX][0-9a-fA-F]{1,2}')
empty [ \n\r\t]

%%
"int"|"float"|"char" { yylval = init_node("TYPE",TOKEN_WITH_VALUE,yytext,yylineno ); return TYPE; }
"if"      { yylval = init_node( "IF", TOKEN_WITH_VALUE, "if", yylineno); return IF; }
"else"    { yylval = init_node( "ELSE", TOKEN_WITH_VALUE,"else", yylineno ); return ELSE; }
"return"  { yylval = init_node( "RETURN", TOKEN_WITH_VALUE,"return", yylineno); return RETURN; }
"("       { yylval = init_node( "LP", TOKEN_WITH_VALUE, "(", yylineno); return LP; }
")"       { yylval = init_node( "LP", TOKEN_WITH_VALUE, ")", yylineno ); return RP; }
";"       { yylval = init_node( "SEMI", TOKEN_WITH_VALUE, ",", yylineno ); return SEMI; }
","       { yylval = init_node( "COMMA", TOKEN_WITH_VALUE, "comma", yylineno); return COMMA; }
{identifier} { yylval = init_node( "ID",TOKEN_WITH_VALUE,yytext,yylineno ); return ID;}
{char}       { yylval = init_node( "CHAR",TOKEN_WITH_VALUE,yytext,yylineno); return CHAR; }

{empty} {}
. { printf("lexical error: %s\n", yytext); }

%%
