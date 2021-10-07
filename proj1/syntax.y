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
Program: ExtDefList { 
    root = $$ = init_node("Program", TOKEN_WITH_VALUE, "Program", @$.first_line ); insert_children($$,1,$1);} 
        ;
ExtDefList: %empty {$$ = init_node("ExtDefList",NON_TERMINAL, NULL, @$.first_line); }
        |  ExtDef ExtDefList { $$ = init_node("ExtDefList",TOKEN_WITH_VALUE,"ExtDefList", @$.first_line); 
                               insert_children($$, 2, $1, $2);}  //需要处理 最后一个extdef不打印
        ;
ExtDef: Specifier ExtDecList SEMI { $$ = init_node("ExtDef", TOKEN_WITH_VALUE,"ExtDef", @$.first_line); 
                                    insert_children($$, 2, $1, $2); }
        | Specifier SEMI { $$ = init_node("ExtDef", TOKEN_WITH_VALUE,"ExtDef", @$.first_line); 
                                    insert_children($$, 2, $1, $2); }
        | Specifier FunDec CompSt { $$ = init_node("ExtDef", TOKEN_WITH_VALUE,"ExtDef", @$.first_line); 
                                    insert_children($$, 3, $1, $2, $3); }
        ;
ExtDecList: VarDec { $$ = init_node("ExtDecList", TOKEN_WITH_VALUE,"ExtDecList", @$.first_line); 
                                    insert_children($$, 1, $1); }
        | VarDec COMMA ExtDecList { $$ = init_node("ExtDecList", TOKEN_WITH_VALUE,"ExtDecList", @$.first_line); 
                                    insert_children($$, 3, $1, $2, $3); }
        ;
Specifier: TYPE {  $$ = init_node("Specifier",TOKEN_WITH_VALUE,"Specifier",@$.first_line);
                    insert_children($$,1,$1); }
        ;
VarDec: ID { $$ = init_node("VarDec", TOKEN_WITH_VALUE, "VarDec", @$.first_line);
                    insert_children($$, 1, $1); }
        | VarDec LB INT RB { $$ = init_node("VarDec", TOKEN_WITH_VALUE, "VarDec", @$.first_line); 
                                    insert_children($$, 4, $1, $2, $3, $4); }
        ;
FunDec: ID LP VarList RP {}
        | ID LP RP {  $$ = init_node("FunDec",TOKEN_WITH_VALUE,"FunDec",@$.first_line);
                      insert_children($$, 3, $1, $2, $3);
                   }
        ;
VarList: ParamDec COMMA VarList {  $$ = init_node("VarList",TOKEN_WITH_VALUE,"VarList",@$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | ParamDec {  $$ = init_node("VarList",TOKEN_WITH_VALUE,"VarList",@$.first_line);
                      insert_children($$, 1, $1);}
        ;
ParamDec: Specifier VarDec {  $$ = init_node("ParamDec",TOKEN_WITH_VALUE,"ParamDec",@$.first_line);
                      insert_children($$, 2, $1, $2);}
        ;
CompSt: LC RC {  $$ = init_node("CompSt",TOKEN_WITH_VALUE,"CompSt",@$.first_line);
                      insert_children($$, 2, $1, $2);}
        | LC DefList StmtList RC {  $$ = init_node("CompSt",TOKEN_WITH_VALUE,"CompSt",@$.first_line);
                      insert_children($$, 4, $1, $2, $3, $4);}
        ;
StmtList: Stmt StmtList {  $$ = init_node("StmtList",TOKEN_WITH_VALUE,"StmtList",@$.first_line);
                      insert_children($$, 2, $1, $2);}
        | %empty {$$ = init_node("StmtList",NON_TERMINAL, NULL, @$.first_line); }
        ;
Stmt: Exp SEMI {  $$ = init_node("Stmt",TOKEN_WITH_VALUE, "Stmt", @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | CompSt {  $$ = init_node("Stmt",TOKEN_WITH_VALUE, "Stmt", @$.first_line);
                      insert_children($$, 1, $1);}
        | RETURN Exp SEMI {  $$ = init_node("Stmt",TOKEN_WITH_VALUE, "Stmt", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | IF LP Exp RP Stmt ELSE Stmt {  $$ = init_node("Stmt",TOKEN_WITH_VALUE, "Stmt", @$.first_line);
                      insert_children($$, 6, $1, $2, $3, $4, $5, $6);}
        | IF LP Exp RP Stmt {  $$ = init_node("Stmt",TOKEN_WITH_VALUE, "Stmt", @$.first_line);
                      insert_children($$, 4, $1, $2, $3, $4);}
        | WHILE LP Exp RP Stmt {  $$ = init_node("Stmt",TOKEN_WITH_VALUE, "Stmt", @$.first_line);
                      insert_children($$, 4, $1, $2, $3, $4);}
        ;

/* local definition */
DefList: Def DefList {  $$ = init_node("DefList",TOKEN_WITH_VALUE, "DefList", @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | %empty {$$ = init_node("DefList",NON_TERMINAL, NULL, @$.first_line); }
        ;
Def: Specifier DecList SEMI {  $$ = init_node("Def",TOKEN_WITH_VALUE, "Def", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        ;
DecList: Dec {  $$ = init_node("DecList",TOKEN_WITH_VALUE, "DecList", @$.first_line);
                      insert_children($$, 1, $1);}
        | Dec COMMA DecList {  $$ = init_node("DecList",TOKEN_WITH_VALUE, "DecList", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        ;
Dec: VarDec
        | VarDec ASSIGN Exp {  $$ = init_node("Dec",TOKEN_WITH_VALUE, "Dec", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        ;
Exp: Exp ASSIGN Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp AND Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp OR Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp LT Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp LE Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp GT Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp GE Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp NE Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp EQ Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp PLUS Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp MINUS Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp MUL Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp DIV Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | LP Exp RP {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | MINUS Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | NOT Exp {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | ID LP Args RP
        | ID LP RP {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp LB Exp RB {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3, $4);}
        | Exp DOT ID {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | ID {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 1, $1);}
        | INT {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 1, $1);}
        | FLOAT {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 1, $1);}
        | CHAR {  $$ = init_node("Exp",TOKEN_WITH_VALUE, "Exp", @$.first_line);
                      insert_children($$, 1, $1);}
        ;
Args: Exp COMMA Args {  $$ = init_node("Args",TOKEN_WITH_VALUE, "Args", @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp {  $$ = init_node("Args",TOKEN_WITH_VALUE, "Args", @$.first_line);
                      insert_children($$, 1, $1);}
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