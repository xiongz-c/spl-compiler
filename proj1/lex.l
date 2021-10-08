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
    void multiple_line();
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
"int"|"float"|"char" { yylval = init_node( "TYPE", NON_TERMINAL, yytext, yylineno ); return TYPE; }
"if"      { yylval = init_node( "IF", NON_TERMINAL, yytext, yylineno ); return IF; }
"else"    { yylval = init_node( "ELSE", NON_TERMINAL, yytext, yylineno ); return ELSE; }
"struct"  { yylval = init_node( "STRUCT", NON_TERMINAL, yytext, yylineno ); return STRUCT; }
"while"   { yylval = init_node( "WHILE", NON_TERMINAL, yytext, yylineno ); return WHILE; }
"for"     { yylval = init_node( "FOR", NON_TERMINAL, yytext, yylineno ); return FOR; }
"return"  { yylval = init_node( "RETURN", NON_TERMINAL, yytext, yylineno ); return RETURN; }

"("       { yylval = init_node( "LP", NON_TERMINAL, yytext, yylineno ); return LP; }
")"       { yylval = init_node( "RP", NON_TERMINAL, yytext, yylineno ); return RP; }
"["       { yylval = init_node( "LB", NON_TERMINAL, yytext, yylineno ); return LB; }
"]"       { yylval = init_node( "RB", NON_TERMINAL, yytext, yylineno ); return RB; }
"{"       { yylval = init_node( "LC", NON_TERMINAL, yytext, yylineno ); return LC; }
"}"       { yylval = init_node( "RC", NON_TERMINAL, yytext, yylineno ); return RC; }

"+"       { yylval = init_node("PLUS", NON_TERMINAL, yytext, yylineno ); return PLUS; }
"-"       { yylval = init_node("MINUS", NON_TERMINAL, yytext, yylineno ); return MINUS; }
"*"       { yylval = init_node("MUL", NON_TERMINAL, yytext, yylineno ); return MUL; }
"/"       { yylval = init_node("DIV", NON_TERMINAL, yytext, yylineno ); return DIV; }
"&&"      { yylval = init_node("AND", NON_TERMINAL, yytext, yylineno ); return AND; }
"||"      { yylval = init_node("OR", NON_TERMINAL, yytext, yylineno ); return OR; }
"!"       { yylval = init_node("NOT", NON_TERMINAL, yytext, yylineno ); return NOT; }
"="       { yylval = init_node("ASSIGN", NON_TERMINAL, yytext, yylineno ); return ASSIGN; }
"<"       { yylval = init_node("LT", NON_TERMINAL, yytext, yylineno ); return LT; }
"<="      { yylval = init_node("LE", NON_TERMINAL, yytext, yylineno ); return LE; }
">"       { yylval = init_node("GT", NON_TERMINAL, yytext, yylineno ); return GT; }
">="      { yylval = init_node("GE", NON_TERMINAL, yytext, yylineno ); return GE; }
"!="      { yylval = init_node("NE", NON_TERMINAL, yytext, yylineno ); return NE; }
"=="      { yylval = init_node("EQ", NON_TERMINAL, yytext, yylineno ); return EQ; }

"."       { yylval = init_node( "DOT", NON_TERMINAL, yytext, yylineno ); return DOT; }
";"       { yylval = init_node( "SEMI", NON_TERMINAL, yytext, yylineno ); return SEMI; }
","       { yylval = init_node( "COMMA", NON_TERMINAL, yytext, yylineno ); return COMMA; }

"\n" { yycolno = 1; }
"//" { char c; while((c=input()) != '\n'); }
"/*" { multiple_line(); }

{integer}    { yylval = init_node("INT", NON_TERMINAL, yytext, yylineno ); return INT; }
{float}      { yylval = init_node("FLOAT", NON_TERMINAL, yytext, yylineno ); return FLOAT; }
{identifier} { yylval = init_node( "ID",NON_TERMINAL, yytext, yylineno ); return ID;}
{char}       { yylval = init_node( "CHAR",NON_TERMINAL,yytext,yylineno ); return CHAR; }

{empty} {}
. { printf("lexical error: %s\n", yytext); }

%%


void multiple_line() {
    char c;
    while(1)
    {
        switch(input()){
        case '\0':
            fprintf(stderr, "Unclosed comment\n");
            exit(-1);
        case '*':
            if ((c = input()) == '/')
                return;
            unput(c);
            break;
        default:
            break;
        }
    }
}