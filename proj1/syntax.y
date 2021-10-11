%{
    #define YYSTYPE ast_node *
    #include "lex.yy.c"
    void syntax_error(int line , char * msg){
        existError = 1;
        printf("Error type B at Line %d: %s\n",line,msg);
    }
    ast_node *root;
    
    void yyerror(const char*);
%}
%token TYPE STRUCT 
%token IF ELSE WHILE FOR RETURN 
%token LC RC SEMI COMMA 
%token INT FLOAT CHAR ID FTOKEN 
%token FAKEID FAKEOP
%nonassoc THEN 
%nonassoc ELSE 
%nonassoc FTOKEN 
%right ASSIGN 
%left OR AND LT LE GT GE EQ NE 
%left PLUS MINUS 
%left MUL DIV 
%right NOT 
%left LP RP LB RB DOT 

%%
Program: ExtDefList { 
    root = $$ = init_node("Program", NON_TERMINAL, NULL, @$.first_line ); insert_children($$,1,$1);} 
        ;
ExtDefList: %empty {$$ = init_node("ExtDefList",NON_TERMINAL, NULL, @$.first_line); }
        |  ExtDef ExtDefList { $$ = init_node("ExtDefList",NON_TERMINAL, NULL, @$.first_line); 
                               insert_children($$, 2, $1, $2);}
        ;
ExtDef: Specifier ExtDecList SEMI { $$ = init_node("ExtDef", NON_TERMINAL, NULL, @$.first_line); 
                                    insert_children($$, 3, $1, $2, $3); }
        | Specifier SEMI { $$ = init_node("ExtDef", NON_TERMINAL, NULL, @$.first_line); 
                                    insert_children($$, 2, $1, $2); }
        | Specifier FunDec CompSt { $$ = init_node("ExtDef", NON_TERMINAL, NULL, @$.first_line); 
                                    insert_children($$, 3, $1, $2, $3); }
        | Specifier ExtDecList error { syntax_error(@2.last_line , "Missing semicolon\';\'");}
        | Specifier ExtDecList SEMI SEMI error { syntax_error(@2.last_line , "Multiple \';\'");}
        | Specifier error SEMI {syntax_error(@3.last_line , "Definition not match rules");}
        ;
ExtDecList: VarDec { $$ = init_node("ExtDecList", NON_TERMINAL, NULL, @$.first_line); 
                                    insert_children($$, 1, $1); }
        | VarDec COMMA ExtDecList { $$ = init_node("ExtDecList", NON_TERMINAL, NULL, @$.first_line); 
                                    insert_children($$, 3, $1, $2, $3); }
        ;
Specifier: TYPE {  $$ = init_node("Specifier", NON_TERMINAL, NULL, @$.first_line);
                    insert_children($$, 1, $1); }
        | StructSpecifier {  $$ = init_node("Specifier", NON_TERMINAL, NULL, @$.first_line);
                    insert_children($$, 1, $1); }
        ;
StructSpecifier: STRUCT ID LC DefList RC { $$ = init_node("StructSpecifier", NON_TERMINAL, NULL, @$.first_line);
                                        insert_children($$, 5, $1, $2, $3, $4, $5); }
                | STRUCT ID { $$ = init_node("StructSpecifier", NON_TERMINAL, NULL, @$.first_line);
                                        insert_children($$, 2, $1, $2); }
                | STRUCT FAKEID error { existError = 1;}
                | STRUCT FAKEID LC DefList RC error { existError = 1;}
                | STRUCT ID LC DefList error {syntax_error(@4.last_line , "Missing \'}\'"); }
        ;
VarDec: ID { $$ = init_node("VarDec", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 1, $1); }
        | VarDec LB INT RB { $$ = init_node("VarDec", NON_TERMINAL, NULL, @$.first_line); 
                        insert_children($$, 4, $1, $2, $3, $4); }
        | FAKEID {existError = 1;}
        | VarDec LB INT error {syntax_error(@3.last_line,"Missing \']\'");}
        | VarDec INT RB error {syntax_error(@3.last_line,"Missing \'[\'");}
        ;
FunDec: ID LP VarList RP { $$ = init_node("FunDec",NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 4, $1, $2, $3, $4); }
        | ID LP RP {  $$ = init_node("FunDec", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 3, $1, $2, $3);
                   }
        | ID LP error { syntax_error(@1.first_line , "Missing closing parenthesis \')\'");}
        | ID LP VarList error { syntax_error(@1.first_line , "Missing closing parenthesis \')\'");}
        | FAKEID LP VarList RP {existError = 1;}
        | FAKEID LP RP {existError = 1;}
        | FAKEID LP VarList error { syntax_error(@1.first_line , "Missing closing parenthesis \')\'");}
        | FAKEID LP error { syntax_error(@1.first_line , "Missing closing parenthesis \')\'");}
        ;
VarList: ParamDec COMMA VarList {  $$ = init_node("VarList",NON_TERMINAL,NULL,@$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | ParamDec {  $$ = init_node("VarList",NON_TERMINAL,NULL,@$.first_line);
                      insert_children($$, 1, $1);}
        ;
ParamDec: Specifier VarDec {  $$ = init_node("ParamDec",NON_TERMINAL,NULL,@$.first_line);
                      insert_children($$, 2, $1, $2);}
        ;
CompSt: LC RC {  $$ = init_node("CompSt", NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | LC DefList error {syntax_error(@3.last_line , "Missing \'}\'");}
        | LC DefList StmtList RC {  $$ = init_node("CompSt", NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 4, $1, $2, $3, $4);}   
        ;
StmtList: Stmt StmtList {  $$ = init_node("StmtList", NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | %empty { $$ = init_node("StmtList", NON_TERMINAL, NULL, @$.first_line); }
        ;
Stmt: Exp SEMI {  $$ = init_node("Stmt", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 2, $1, $2);}
        | CompSt {  $$ = init_node("Stmt", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 1, $1);}
        | RETURN Exp SEMI {  $$ = init_node("Stmt", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 3, $1, $2, $3);}
        | IF LP Exp RP Stmt ELSE Stmt {  $$ = init_node("Stmt", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 7, $1, $2, $3, $4, $5, $6, $7);}
        | IF LP Exp RP Stmt %prec THEN {  $$ = init_node("Stmt", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 5, $1, $2, $3, $4, $5);}
        | WHILE LP Exp RP Stmt {  $$ = init_node("Stmt", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 5, $1, $2, $3, $4, $5);}
        | FOR LP ForVarList RP Stmt { {$$ = init_node("Stmt", NON_TERMINAL, NULL, @$.first_line);
                        insert_children($$, 5, $1, $2, $3, $4, $5);} }
        | IF LP Exp error Stmt ELSE Stmt  { syntax_error(@3.last_line, "Missing closing parenthesis \')\'"); }
        | IF LP Exp error Stmt %prec THEN { syntax_error(@3.last_line, "Missing closing parenthesis \')\'"); }
        | RETURN Exp error { syntax_error(@1.first_line , "Missing semicolon \';\'");}
        | Exp error { syntax_error(@1.first_line , "Missing semicolon \';\'"); }
        | Exp SEMI SEMI error { syntax_error(@3.first_line , "Multiple \';\'"); }
        ;

/* local definition */
DefList: Def DefList {  $$ = init_node("DefList",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | %empty {$$ = init_node("DefList",NON_TERMINAL, NULL, @$.first_line); }
        | DefList error Def  { syntax_error(@2.first_line, "Missing specifier"); }
        ;
Def: Specifier DecList SEMI {  $$ = init_node("Def",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Specifier DecList SEMI SEMI error {syntax_error(@1.first_line , "Multiple \';\'");}
        | Specifier DecList error  {syntax_error(@1.first_line , "Missing semicolon \';\'");}
        ;
DecList: Dec {  $$ = init_node("DecList",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 1, $1);}
        | Dec COMMA DecList {  $$ = init_node("DecList",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        ;
Dec: VarDec {  $$ = init_node("Dec",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 1, $1);}
        | VarDec ASSIGN Exp {  $$ = init_node("Dec",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | VarDec LT Exp error {syntax_error(@2.first_line , "Wrong Assign Operation");}
        | VarDec LE Exp error {syntax_error(@2.first_line , "Wrong Assign Operation");}
        | VarDec GT Exp error {syntax_error(@2.first_line , "Wrong Assign Operation");}
        | VarDec GE Exp error {syntax_error(@2.first_line , "Wrong Assign Operation");}
        | VarDec NE Exp error {syntax_error(@2.first_line , "Wrong Assign Operation");}
        | VarDec EQ Exp error {syntax_error(@2.first_line , "Wrong Assign Operation");}
        ;
Exp: Exp ASSIGN Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp AND Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp OR Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp LT Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp LE Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp GT Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp GE Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp NE Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp EQ Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp PLUS Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp MINUS Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp MUL Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp DIV Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | LP Exp RP {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | MINUS Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | NOT Exp {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 2, $1, $2);}
        | ID LP Args RP {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 4, $1, $2, $3, $4);}
        | ID LP RP {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp LB Exp RB {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 4, $1, $2, $3, $4);}
        | Exp DOT ID {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | ID {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 1, $1);}
        | INT {  $$ = init_node("Exp",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 1, $1);}
        | FLOAT {  $$ = init_node("Exp", NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 1, $1);}
        | CHAR {  $$ = init_node("Exp", NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 1, $1);}
        | FTOKEN error { existError = 1; }
        | FAKEID {existError = 1; }
        | Exp FTOKEN Exp error  { existError = 1; }
        | Exp FAKEOP Exp error  { existError = 1; }
        | ID LP Args error { syntax_error(@1.first_line , "Missing closing parenthesis \')\'"); }
        ;
ForVarList: DecList SEMI Exp SEMI Args  { $$ = init_node("ForVarList", NON_TERMINAL, NULL, @$.first_line);
                                                insert_children($$, 5, $1, $2, $3, $4, $5);}
        ;
Args: Exp COMMA Args {  $$ = init_node("Args",NON_TERMINAL, NULL, @$.first_line);
                      insert_children($$, 3, $1, $2, $3);}
        | Exp {  $$ = init_node("Args",NON_TERMINAL, NULL, @$.first_line);
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
    if(!existError)print_tree(root,0);
    return 0;
}