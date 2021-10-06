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
%token TYPE STRUCT IF ELSE WHILE FOR RETURN 
%token LC RC SEMI COMMA 
%token INT FLOAT CHAR ID FTOKEN 
%nonassoc THEN 
%nonassoc ELSE 
%right ASSIGN 
%left OR 
%left AND 
%left LT LE GT GE EQ NE 
%left PLUS MINUS 
%left MUL DIV 
%right NOT 
%left LP RP LB RB DOT 
%nonassoc FTOKEN 
%%
Program : ExtDefList { 
    root = $$ = init_node("Program", TOKEN_WITH_VALUE, "Program", @$.first_line ); insert_children($$,1,$1);} 
    ;
ExtDefList : %empty {$$ = init_node("ExtDefList",TOKEN_WITH_VALUE,NULL, @$.first_line); }
        |  ExtDef ExtDefList { $$ = init_node("ExtDefList",TOKEN_WITH_VALUE,"ExtDefList", @$.first_line); 
                               insert_children($$,1,$1);}  //可能是错的
    ;
ExtDef : Specifier ExtDecList SEMI {}
        | Specifier SEMI {}
        | Specifier FunDec CompSt { $$ = init_node("ExtDef",TOKEN_WITH_VALUE,"ExtDef",@$.first_line); 
                                    insert_children($$,3,$1,$2,$3);
                                  }
    ;
ExtDecList : VarDec {}
        | VarDec COMMA ExtDecList {}
    ;
Specifier : TYPE {  $$ = init_node("Specifier",TOKEN_WITH_VALUE,"Specifier",@$.first_line);
                    insert_children($$,1,$1);
                 }
    ;
VarDec : ID {}
        | VarDec LB INT RB {}
    ;
FunDec : ID LP VarList RP {}
        | ID LP RP {  $$ = init_node("FunDec",TOKEN_WITH_VALUE,"FunDec",@$.first_line);
                      insert_children($$,3,$1,$2,$3);
                   }
    ;
VarList : ParamDec COMMA VarList {}
        | ParamDec {}
    ;
ParamDec : Specifier VarDec {}
    ;
CompSt : LC RC {  $$ = init_node("CompSt",TOKEN_WITH_VALUE,"CompSt",@$.first_line);
                      insert_children($$,2,$1,$2);}
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
    //printf("%s\n",root->children[0]->name);

    print_tree(root,0);
    return 0;
}
// make clean;make