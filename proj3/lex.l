%{
    #include "semantic.hpp"
    #include "tac.hpp"
    #include "syntax.tab.h"
    #define YY_USER_ACTION \
        yylloc.first_line = yylineno; \
        yylloc.first_column = yycolno; \
        yylloc.last_line = yylineno; \
        yylloc.last_column = yycolno + yyleng; \
        yycolno += yyleng; \

    int yycolno = 1;

    int existError = 0;
    void lexical_error(int line, string msg){
        printf("Error type A at Line %d: unknown lexeme %s\n",line,msg.c_str());
        existError = 1;
    }
%}
%option yylineno
%option noyywrap
digit         [0-9]
hexDigit     [0-9a-fA-F]
letter        [A-Za-z]
_letter       [a-zA-Z_]

hexChar '\\[xX]{hexDigit}{1,2}'

fakeId     {digit}+({letter}|_)({digit}|{letter}|_)*
identifier ({letter}|_)({digit}|{letter}|_)*

dec 0|[1-9]{digit}*
hex 0[xX](0|[1-9a-fA-F]{hexDigit}*)
integer ({dec}|{hex})
frac \.{digit}+
float {dec}{frac}
char ('[ -~]')|{hexChar}
fakeChar '\\[xX].+'|'.{3,}'
fakeInt 0[0-9]+
fakeFloat {digit}+\.{digit}+
empty [ \n\r\t]

%%
"int"|"float"|"char" {yylval = new ast_node( "TYPE", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return TYPE; }
"if"      { yylval = new ast_node( "IF", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return IF; }
"else"    { yylval = new ast_node( "ELSE", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return ELSE; }
"struct"  { yylval = new ast_node( "STRUCT", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return STRUCT; }
"while"   { yylval = new ast_node( "WHILE", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return WHILE; }
"for"     { yylval = new ast_node( "FOR", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return FOR; }
"return"  { yylval = new ast_node( "RETURN", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno );return RETURN; }

"("       { yylval = new ast_node( "LP", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return LP; }
")"       { yylval = new ast_node( "RP", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return RP; }
"["       { yylval = new ast_node( "LB", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return LB; }
"]"       { yylval = new ast_node( "RB", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return RB; }
"{"       { yylval = new ast_node( "LC", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return LC; }
"}"       { yylval = new ast_node( "RC", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return RC; }

"+"       { yylval = new ast_node("PLUS", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return PLUS; }
"-"       { yylval = new ast_node("MINUS", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return MINUS; }
"*"       { yylval = new ast_node("MUL", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return MUL; }
"/"       { yylval = new ast_node("DIV", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return DIV; }
"&&"      { yylval = new ast_node("AND", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return AND; }
"||"      { yylval = new ast_node("OR", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return OR; }
"!"       { yylval = new ast_node("NOT", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return NOT; }
"="       { yylval = new ast_node("ASSIGN", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return ASSIGN; }
"<"       { yylval = new ast_node("LT", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return LT; }
"<="      { yylval = new ast_node("LE", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return LE; }
">"       { yylval = new ast_node("GT", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return GT; }
">="      { yylval = new ast_node("GE", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return GE; }
"!="      { yylval = new ast_node("NE", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return NE; }
"=="      { yylval = new ast_node("EQ", TERMINAL_WITHOUT_ATTRIBUTE, yytext, yylineno ); return EQ; }

"."       { yylval = new ast_node( "DOT", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return DOT; }
";"       { yylval = new ast_node( "SEMI", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return SEMI; }
","       { yylval = new ast_node( "COMMA", TERMINAL_WITHOUT_ATTRIBUTE, "", yylineno ); return COMMA; }

"\n" { yycolno = 1; }
"//" { char c; while(( c=yyinput()) != '\n'); }

{integer}    { yylval = new ast_node( "INT", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return INT; }
{float}      { yylval = new ast_node( "FLOAT", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return FLOAT; }
{identifier} { yylval = new ast_node( "ID", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return ID;}
{char}       { yylval = new ast_node( "CHAR", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return CHAR; }

{empty} {}
{fakeId}  { lexical_error(yylineno,yytext); return FAKEID;}
"^"|"/="|"*="|"+="|"-=" {lexical_error(yylineno,yytext); return FAKEOP;}
.|{fakeChar}|{fakeInt}|{fakeFloat} {lexical_error(yylineno,yytext);  return FTOKEN;}

%%