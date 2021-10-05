%{
    #include "lex.yy.c"
%}
%token IF ELSE 
%token INT FLOAT CHAR
%token TYPE RETURN 
%token LP RP
%token LC RC
%token ExtDefList
%token ID SEMI COMMA

%%
Program: ExtDefList { };

%%

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