%{
    #define YYSTYPE ast_node *
    #include "lex.yy.c"
    #define syntax_error(line, msg) \
            flag = 0; \
            printf("Error type B at Line %d: %s\n", line, msg);

    ast_node *root;
    int flag = 1;
    void yyerror(const char*);
%}
%token IF ELSE 
%token INT FLOAT CHAR
%token TYPE RETURN 
%token LP RP
%token LC RC
%token ID SEMI COMMA
%token LB RB
%%
Program : ExtDefList {printf("%d",@$.first_line);} 
    ;
ExtDefList :
        |  ExtDef ExtDefList {}
    ;
ExtDef : Specifier ExtDecList SEMI {}
        | Specifier SEMI {}
        | Specifier FunDec CompSt {}
    ;
ExtDecList : VarDec {}
        | VarDec COMMA ExtDecList {}
    ;
Specifier : TYPE {}
    ;
VarDec : ID {}
        | VarDec LB INT RB {}
    ;
FunDec : ID LP VarList RP {}
        | ID LP RP {}
    ;
VarList : ParamDec COMMA VarList {}
        | ParamDec {}
    ;
ParamDec : Specifier VarDec {}
    ;
CompSt : LC  RC {}
    ;

%%

void yyerror(const char *s) {}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(-1);
    } else if (!(yyin = fopen(argv[1], "r"))) {
        perror(argv[1]);
        exit(-1);
    }
    yyparse();
    return 0;
}
// make clean;make