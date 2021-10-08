typedef enum token_type{
    NON_TERMINAL,
    TERMINAL_WITH_ATTRIBUTE,
    TERMINAL_WITHOUT_ATTRIBUTE,
}token_type;


typedef struct ast_node{
    char* name;
    token_type type;
    char* value;
    int line_num;
    int children_num;
    struct ast_node **children;
}ast_node;

char * str_copy(char * my_str);  
ast_node *init_node(char *ast_name, token_type _type, char *ast_value, int line_num);
void print_node(ast_node *node);
void insert_children(ast_node *, int, ...);
void print_tree(ast_node *root, int height);