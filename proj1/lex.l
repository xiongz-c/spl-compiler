%{
    // just let you know you have macros!
    // C macro tutorial in Chinese: http://c.biancheng.net/view/446.html
    #define EXIT_OK 0
    #define EXIT_FAIL 1
    // global variables
    int chars = 0;
    int words = 0;
    int lines = 0;
   
    
%}
%option yylineno
WhiteSpace [ \t\r\a]+
Digit      [0-9]
letter     [a-zA-Z]
decimal    0|[1-9]{Digit}*


%%
{decimal} {printf("got decimal\n");}
{WhiteSpace} {printf("got white\n");}
{letter}+ { words++; chars+=strlen(yytext); }
\n { printf("line:%d\n",yylineno); chars++; lines++; }
. { chars++; }

%%
int main(int argc, char **argv){
    char *file_path;
    if(argc < 2){
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return EXIT_FAIL;
    } else if(argc == 2){
        file_path = argv[1];
        if(!(yyin = fopen(file_path, "r"))){
            perror(argv[1]);
            return EXIT_FAIL;
        }
        yylex();
	printf("%-8s%-8s%-8s%s\n", "#lines", "#words", "#chars", "file path");
        printf("%-8d%-8d%-8d%s\n", lines, words, chars, file_path);
        return EXIT_OK;
    } else{
        fputs("Too many arguments! Expected: 2.\n", stderr);
        return EXIT_FAIL;
    }
}
