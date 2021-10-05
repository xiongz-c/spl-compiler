typedef enum token_type
{
    NON_TERMINAL,
    TOKEN_WITH_VALUE,
    TOKEN_WITHOUT_VALUE
}token_type;


typedef struct ast_node
{
    char* name;
    token_type type;
    char* value;
    int line_num;
    struct ast_node *child;
    struct ast_node *sibling;
}ast_node;

ast_node *init_node(char *ast_name, token_type _type, char *ast_value, int line_num);
void print_node(ast_node *node);
