#ifndef _SEMANTIC_
#define _SEMANTIC_
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <utility>
#include "ast.hpp"

using namespace std;

#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

unordered_map<int,string> error_info =
        {
                {1,"undefined variable:"},
                {2,"undefined function:"},
                {3,"redefined variable:"},
                {4,"redefined function:"},
        };


/*
 *=======================
 *=== Type declaration===
 *=======================
*/
class Type{
public:
    string name;
    int lVal;
    void setName(string s){
        this->name = s;
    }
    virtual ~Type() = default;
};

class PrimitiveType: public Type{
public:
    enum {INT, FLOAT, CHAR} primitive;
    PrimitiveType(const string& primitive){
        this->lVal = 0;
        this->name = "Primitive_"+primitive;
        if (primitive == "int"){
            this->primitive = INT;
        }else if(primitive == "float") {
            this->primitive = FLOAT;
        }else if(primitive == "char"){
            this->primitive = CHAR;
        }
    }
};

class ArrayType: public Type{
public:
    Type * base;
    int size;
    ArrayType(Type* base, int size): base(base), size(size) {}
};

class Field{
public:
    string name;
    Type * type;
    Field(string name, Type *type1): name(name), type(type1) {}
};

class StructureType: public Type{
public:
    vector<Field*> fields;
    int field_size;

    void pushField(Field* field){
        this->fields.push_back(field);
    }
};

class SymbolElement{
public:
    string value;
    Type* type;
    int scope_num;
    int line_num;
    enum {VARIABLE, FUNCTION, STRUCTURE, EXPRESSION} attribute;

    SymbolElement(string value, Type * type, int scope_num, int line_num, string attr): value(value), type(type), line_num(line_num), scope_num(scope_num) {
        if (attr == "VAR"){
            this->attribute = VARIABLE;
        }else if (attr == "FUNC"){
            this->attribute = FUNCTION;
        }else if (attr == "STRUCT") {
            this->attribute = STRUCTURE;
        }else if (attr == "EXP") {
            this->attribute = EXPRESSION;
        }
    }
};


class SymbolTable{
public:
    vector<string> key_list;
    unordered_multimap<string,SymbolElement*> symbolTableInstance;
    int scope;

    bool insertEntry(string id, SymbolElement* entry){
        if (this->symbolTableInstance.find(id) != this->symbolTableInstance.end()){
            return false;
        }
        this->symbolTableInstance.emplace(id,entry);
        return true;
    }

    SymbolElement* searchEntry(string name, string type){
        string id = type+"_"+name;
        auto item = this->symbolTableInstance.find(id);
        if(item == symbolTableInstance.end()){
            return nullptr;
        }else{
            return item->second;
        }
    }

};


/*
 * Global Var declaration
 * */
SymbolTable symbolTable;



/*
 * ===================================
 * ===Analysis function declaration===
 * ===================================
 * */
void semanticEntry(ast_node*);
void extDefListEntry(ast_node*);
void extDefEntry(ast_node*);
Type* specifierEntry(ast_node*);
Type* structSpecifierEntry(ast_node*);
void varDecEntry(ast_node*, Type*);
void funDecEntry(ast_node*, Type*);
Type* typeEntry(ast_node*);
vector<Type*> varListEntry(ast_node*);
Type* paramDecEntry(ast_node*);
Type* varListLoopEntry(ast_node*);
void compStEntry(ast_node*);
pair<string, SymbolElement*> createTableInfo(ast_node*,  Type*, string);
void defEntry(ast_node* node);
void defListEntry(ast_node* node);
void decListEntry(ast_node* node, Type*);
void decEntry(ast_node* node,Type*);
Type* ExpressionEntry(ast_node*);

void reportError(int type,int line_num,string diy){
    cout << "Error type " << type << " at Line " << line_num <<": " << error_info[type] << diy <<endl;
}

void semanticEntry(ast_node* root){
    if (root->children_num <= 0 )return;
    D(cerr << ">> root children size : " <<root->children.size() << endl;)
    extDefListEntry(root->children[0]);
    //extDefListEntry(root->children[0]);
}


void extDefListEntry(ast_node* node){
    if(node->children_num <= 0) return; // some leaf nodes have no children, which will lead to break down
    extDefEntry(node->children[0]);
    if(node->children_num == 1) return;
    else {
        extDefListEntry(node->children[1]);
        return;
    }
}

void extDefEntry(ast_node* node){
    if(node->children_num <= 0)return;
    Type* spec_type = specifierEntry(node->children[0]);
    if(node->children_num >= 2){
        ast_node* child = node->children[1];
        if(child->name == "FunDec" && node->children_num == 3){
            funDecEntry(child, spec_type);
            compStEntry(node->children[2]);
        }
    }
}

void compStEntry(ast_node* node){
    if (node->children_num != 4)return;
    defListEntry(node->children[1]);
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
}

void defListEntry(ast_node* node){
    if(node->children_num <= 0) return;
    defEntry(node->children[0]);
    if(node->children_num >= 2){
        defListEntry(node->children[1]);
    }
}

void defEntry(ast_node* node){
    if(node->children_num != 3)return;
    Type* spec_type = specifierEntry(node->children[0]);
    decListEntry(node->children[1], spec_type);
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
}
void decListEntry(ast_node* node, Type* spec_type){
    if(node->children_num <= 0)return;
    decEntry(node->children[0],spec_type);
    if(node->children_num == 3){
        decListEntry(node->children[2],spec_type);
    }
}

void decEntry(ast_node* node, Type* spec_type){
    if(node->children_num <= 0)return;
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    varDecEntry(node->children[0],spec_type);
    if(node->children_num == 3){
        Type* type = ExpressionEntry(node->children[2]);
        cout << "fucking type : " << type->name << endl;
    }
}

Type* ExpressionEntry(ast_node* node){
    if(node->children_num <= 0 )return nullptr;
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    if(node->children_num == 1 && node->children[0]->name == "ID"){
        SymbolElement * po = symbolTable.searchEntry(node->children[0]->value,"VAR");
        if(po == nullptr){
            reportError(1,node->children[0]->line_num," "+node->children[0]->value);
        }
    }else if (node->children[0]->name == "ID" && node->children_num >= 3){
        SymbolElement * po = symbolTable.searchEntry(node->children[0]->value,"FUNC");
        if(po == nullptr){
            reportError(2,node->children[0]->line_num," "+node->children[0]->value);
        }
    }else if(node->children_num == 1 && (node->children[0]->name == "INT" || node->children[0]->name == "FLOAT" || node->children[0]->name == "CHAR" )){
        string str_type = node->children[0]->name;
        transform(str_type.begin(),str_type.end(),str_type.begin(),::tolower);
        Type* type = new PrimitiveType(str_type);
        return type;
    }else if(node->children[0]->name == "Exp"){
        Type* left_exp_type = ExpressionEntry(node->children[0]);
        D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
        if(node->children_num == 3 && node->children[1]->name == "ASSIGN"){
            Type* right_exp_type = ExpressionEntry(node->children[2]);
        }else if (node->children_num == 3 && (node->children[1]->name == "PLUS"
                                           || node->children[1]->name == "MINUS"
                                           || node->children[1]->name == "MUL"
                                           || node->children[1]->name == "DIV")){
            Type* right_exp_type = ExpressionEntry(node->children[2]);
        }
    }
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
}

//TODO Exp

Type* specifierEntry(ast_node* node){
    if(node->children_num != 1) return nullptr; //Specifier only have one child
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    if(node->children[0]->name == "TYPE"){
        Type * spec_type = typeEntry(node->children[0]);
        return spec_type;
    }
    else if (node->children[0]->name == "StructSpecifier"){
        Type * spec_type = structSpecifierEntry(node->children[0]);
        return spec_type;
    }
}

Type* typeEntry(ast_node* node){
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    Type * res = new PrimitiveType(node->value);
    string name = "Primitive_"+node->value;
    return res;
}

Type* structSpecifierEntry(ast_node* node){
    return nullptr;
}

void funDecEntry(ast_node* node, Type * spec_type){
    if(node->children_num <= 2) return;
    if(node->children[2]->name == "VarList"){
        vector<Type*> parm_types = varListEntry(node->children[2]);
        pair<string, SymbolElement*> info = createTableInfo(node->children[0],spec_type,"FUNC");
        if( !symbolTable.insertEntry(info.first,info.second)){
             reportError(4,node->children[0]->line_num," "+node->children[0]->value);
        }
        D( cerr << "lineno: " << __LINE__ << " parm_types size: " << parm_types.size() << endl;)
    }else {
        pair<string, SymbolElement*> info=  createTableInfo(node->children[0],spec_type,"FUNC");
        if( !symbolTable.insertEntry(info.first,info.second)){
             reportError(4,node->children[0]->line_num," "+node->children[0]->value);
        }
    }

}

vector<Type*> varListEntry(ast_node* node){
    vector<Type*> res;
    if(node->children_num <= 0)return res;
    res.push_back(paramDecEntry(node->children[0]));
    ast_node* begin_node = node;
    while (begin_node->children_num >= 3) {
        begin_node = begin_node->children[2];
        res.push_back(paramDecEntry(begin_node->children[0]));
    }
    return res;
}

Type* paramDecEntry(ast_node* node){
    if(node->children_num <= 0)return nullptr;
    Type* spec_type = specifierEntry(node->children[0]);
    D( cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    varDecEntry(node->children[1], spec_type);
    return spec_type;
}

void varDecEntry(ast_node* node, Type* spec_type){
    if(node->children_num <= 0)return;
    if(node->children[0]->name == "ID"){
        pair<string, SymbolElement*> info = createTableInfo(node->children[0],spec_type,"VAR");
        if (!symbolTable.insertEntry(info.first,info.second)){
            reportError(3,node->children[0]->line_num," "+node->children[0]->value);
        }
    }else if(node->children[0]->name == "VarDec" && node->children_num == 4){
        //TODO solve array
        D( cerr << "lineno: " << __LINE__ << " " << node->children_num << endl;)
    }

}

pair<string, SymbolElement*> createTableInfo(ast_node* node, Type *type, string attr){
    D( cerr << "lineno: " << __LINE__ << " node name : " << node->name << endl;)
    if (attr == "VAR"){
        D( cerr << "lineno: " << __LINE__ << " node value : " << node->value << endl;)
        SymbolElement * element = new SymbolElement(node->value, type, symbolTable.scope, node->line_num, "VAR");
        string id = "VAR_"+node->value;
        D( cerr << "element info: "<< " entry ID: " << id << endl;)
        return make_pair(id,element);
    }else if (attr == "FUNC"){
        //PrimitiveType* primitiveType = dynamic_cast<PrimitiveType*>(type);
        D( cerr << "lineno: " << __LINE__ << " node value : " << node->value << endl;)
        SymbolElement * element = new SymbolElement(node->value, type, symbolTable.scope, node->line_num, "FUNC");
        string id = "FUNC_"+node->value;
        return make_pair(id, element);
    }

}

#endif
