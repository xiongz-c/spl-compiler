#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"

ast_node *init_node(char *ast_name, token_type _type, char *ast_value, int line_num){
    ast_node *n = malloc(sizeof(ast_node));
    n->name = str_copy(ast_name);
    n->type = _type;
    n->value = str_copy(ast_value);
    n->line_num = line_num;
    n->child = NULL;
    n->sibling = NULL;
    printf("node : %s \n", ast_name);
    return n;
}

char * str_copy(char * my_str){
    if (NULL == my_str){
        return NULL;
    }
    char * res = (char*)malloc( strlen(my_str) + 1 );
    strcpy(res, my_str);
    res[strlen(my_str)] = '\0';
    return res;
}

void print_node(ast_node *node){
    
}