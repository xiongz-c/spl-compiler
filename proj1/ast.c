#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"

ast_node *init_node(char *ast_name, token_type _type, char *ast_value, int line_num){
    ast_node *n = malloc(sizeof(ast_node));
    n->name = ast_name;
    n->type = _type;
    n->value = ast_value;
    n->line_num = line_num;
    n->child = NULL;
    n->sibling = NULL;
    return n;
}
void print_node(ast_node *node){
    
}