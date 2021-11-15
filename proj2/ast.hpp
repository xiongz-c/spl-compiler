#include <iostream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

using namespace std;
enum token_type{
    NON_TERMINAL,
    TERMINAL_WITH_ATTRIBUTE,
    TERMINAL_WITHOUT_ATTRIBUTE,
};

class ast_node{
public:
    string name;
    token_type type;
    string value;
    int line_num;
    int children_num;
    vector<ast_node*> children;

    ast_node(const string& _name, token_type _type, const string& _value, int _line_num){
        this->name = _name;
        this->type = _type;
        this->value = _value;
        this->line_num = _line_num;
        this->children_num = 0;
        if( _name =="INT" && _value.size() > 2 &&
            ( _value[1] == 'x' || _value[2] == 'x' || _value[1] == 'X' || _value[2] == 'X') )
        {
            int a = (int)strtol(_value.c_str(), NULL, 0);
            stringstream ss;
            ss << a;
            string tmp_val_str = ss.str();
            this->value = tmp_val_str;
        }
    }

    void insert_children(int num, ...){
        this->children_num = num;
        va_list args;
        va_start(args, num);
        while(num--){
            ast_node* tmp_node = va_arg(args, ast_node*);
            this->children.push_back(tmp_node);
        }
        va_end(args);
    }

    string printNode(){
        string info = "";
        info += ">> Node name : " + this->name + " | value: "+this->value +" | children num: " + to_string(this->children_num) + " ";
        for (auto s : this->children){
            info += " "+ s->name;
        }
        info += "\n";
        return info;
    }
};

void print_tree(ast_node *root, int height){
    if (root == NULL)return;
    if (root->type == NON_TERMINAL && root->children_num == 0){return;}
    for(int i = 0; i < height ; i++){
        printf("  ");
    }
    if (root->type == NON_TERMINAL){
        printf("%s (%d)\n",root->name.c_str(),root->line_num);
    }
    if (root->type == TERMINAL_WITH_ATTRIBUTE)printf("%s: %s\n",root->name.c_str(), root->value.c_str());
    if (root->type == TERMINAL_WITHOUT_ATTRIBUTE)printf("%s\n",root->name.c_str());
    for (int i = 0 ; i < root->children_num ; i++){
        print_tree(root->children[i],height+1);
    }

}

