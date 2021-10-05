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
decInt 0|(-?[1-9][0-9]*)
hexInt (0[xX]0|0[xX][1-9a-fA-F][1-9a-fA-F]{0,7})
int {decInt}|{hexInt}
frac \.[0-9]+
float -?(0|[1-9][0-9]*){frac}
_letter [a-zA-Z_]
id [a-zA-Z_][0-9a-zA-Z_]*
wrong_id [0-9][0-9a-zA-Z_]*
char ('{_letter}')|('\\[xX][0-9a-fA-F]{1,2}')
wrong_char ('.+')

%%

{decInt} {printf("got dec %d\n", atoi(yytext)); }
\n { chars++; lines++; }
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
        return EXIT_OK;
    } else{
        fputs("Too many arguments! Expected: 2.\n", stderr);
        return EXIT_FAIL;
    }
}
