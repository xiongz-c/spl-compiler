%{
    #include "ast.h"
    #include "syntax.tab.h"
    #define YY_USER_ACTION \
        yylloc.first_line = yylineno; \
        yylloc.first_column = yycolno; \
        yylloc.last_line = yylineno; \
        yylloc.last_column = yycolno + yyleng; \
        yycolno += yyleng;
    
    int yycolno = 1;
    int existError = 0;
    void lexical_error(int line, char * msg){
        printf("Error type A at Line %d: unknown lexeme %s\n",line,msg);
        existError = 1;
    }
%}
%option yylineno

digit         [0-9]
hexDigit     [0-9a-fA-F]
letter        [A-Za-z]
_letter       [a-zA-Z_]

hexChar '\\[xX]{hexDigit}{1,2}'

fakeId     {digit}+({letter}|_)({digit}|{letter}|_)*
identifier ({letter}|_)({digit}|{letter}|_)*

dec 0|[1-9]{digit}*
hex 0[xX](0|[1-9a-fA-F]{hexDigit}*)
integer -?({dec}|{hex})
frac \.{digit}+
float -?{dec}{frac}
char ('[ -~]')|{hexChar}
fakeChar '\\[xX].+'|'.{3,}'
fakeInt -?0[0-9]+
fakeFloat -?{digit}+\.{digit}+
empty [ \n\r\t]

%%
"int"|"float"|"char" {yylval = init_node( "TYPE", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return TYPE; }
"if"      { yylval = init_node( "IF", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return IF; }
"else"    { yylval = init_node( "ELSE", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return ELSE; }
"struct"  { yylval = init_node( "STRUCT", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return STRUCT; }
"while"   { yylval = init_node( "WHILE", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return WHILE; }
"for"     { yylval = init_node( "FOR", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return FOR; }
"return"  { yylval = init_node( "RETURN", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno );return RETURN; }

"("       { yylval = init_node( "LP", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return LP; }
")"       { yylval = init_node( "RP", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return RP; }
"["       { yylval = init_node( "LB", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return LB; }
"]"       { yylval = init_node( "RB", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return RB; }
"{"       { yylval = init_node( "LC", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return LC; }
"}"       { yylval = init_node( "RC", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return RC; }

"+"       { yylval = init_node("PLUS", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return PLUS; }
"-"       { yylval = init_node("MINUS", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return MINUS; }
"*"       { yylval = init_node("MUL", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return MUL; }
"/"       { yylval = init_node("DIV", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return DIV; }
"&&"      { yylval = init_node("AND", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return AND; }
"||"      { yylval = init_node("OR", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return OR; }
"!"       { yylval = init_node("NOT", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return NOT; }
"="       { yylval = init_node("ASSIGN", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return ASSIGN; }
"<"       { yylval = init_node("LT", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return LT; }
"<="      { yylval = init_node("LE", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return LE; }
">"       { yylval = init_node("GT", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return GT; }
">="      { yylval = init_node("GE", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return GE; }
"!="      { yylval = init_node("NE", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return NE; }
"=="      { yylval = init_node("EQ", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return EQ; }

"."       { yylval = init_node( "DOT", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return DOT; }
";"       { yylval = init_node( "SEMI", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return SEMI; }
","       { yylval = init_node( "COMMA", TERMINAL_WITHOUT_ATTRIBUTE, NULL, yylineno ); return COMMA; }

"\n" { yycolno = 1; }
"//" { char c; while((c=input()) != '\n'); }

{integer}    { yylval = init_node( "INT", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return INT; }
{float}      { yylval = init_node( "FLOAT", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return FLOAT; }
{identifier} { yylval = init_node( "ID", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return ID;}
{char}       { yylval = init_node( "CHAR", TERMINAL_WITH_ATTRIBUTE, yytext, yylineno ); return CHAR; }

{empty} {}
{fakeId}  { lexical_error(yylineno,yytext); return FAKEID;}
.|{fakeChar}|{fakeInt}|{fakeFloat} {lexical_error(yylineno,yytext); return FTOKEN;}

%%