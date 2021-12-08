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

unordered_map<int, string> error_info =
        {
                {1,  "undefined variable:"},
                {2,  "undefined function:"},
                {3,  "redefine variable:"},
                {4,  "redefine function:"},
                {5,  "unmatching type on both sides of assignment"},
                {6,  "left side in assignment is rvalue"},
                {7,  "binary operation on non-number variables"},
                {8,  "incompatiable return type"},
                {9,  "invalid "},
                {10, "indexing on non-array variable"},
                {11, "invoking non-function variable: "},
                {12, "indexing by non-integer"},
                {13, "accessing with non-struct variable"},
                {14, "no such member"},
                {15, "redefine struct"},
                {16, "only int can do boolean operation"}
        };


/*
 *=======================
 *=== Type declaration===
 *=======================
*/
class Type {
public:
    string name;
    string filed_name;
    int lVal;

    void setName(string s) {
        this->name = s;
    }

    virtual ~Type() = default;
};

class PrimitiveType : public Type {
public:
    enum {
        INT, FLOAT, CHAR
    } primitive;

    PrimitiveType(const string &primitive) {
        this->lVal = 0;
        this->name = "Primitive_" + primitive;
        if (primitive == "int") {
            this->primitive = INT;
        } else if (primitive == "float") {
            this->primitive = FLOAT;
        } else if (primitive == "char") {
            this->primitive = CHAR;
        }
    }
};

class ArrayType : public Type {
public:
    Type *base;
    int size;

    ArrayType(Type *base, int size) : base(base), size(size) {
        this->name = "Array_" + base->name;
    }
};


class StructureType : public Type {
public:
    vector<Type *> fields;
    int field_size;

    void pushField(Type *field) {
        this->fields.push_back(field);
    }

    StructureType(vector<Type *> *vec) {
        for (int i = 0; i < vec->size(); i++) {
            fields.push_back((*vec)[i]);
        }
        field_size = vec->size();
        this->name = "Structure";
    }

};

class SymbolElement {
public:
    string value;
    Type *type;
    int scope_num;
    int line_num;
    enum {
        VARIABLE, FUNCTION, STRUCTURE, EXPRESSION, ARRAY
    } attribute;
    vector<Type *> args;

    SymbolElement(string value, Type *type, int scope_num, int line_num, string attr) : value(value), type(type),
                                                                                        line_num(line_num),
                                                                                        scope_num(scope_num) {
        if (attr == "VAR") {
            this->attribute = VARIABLE;
        } else if (attr == "FUNC") {
            this->attribute = FUNCTION;
        } else if (attr == "STRUCT") {
            this->attribute = STRUCTURE;
        } else if (attr == "EXP") {
            this->attribute = EXPRESSION;
        } else if (attr == "ARRAY") {
            this->attribute = ARRAY;
        }
    }

    SymbolElement(string value, Type *type, int scope_num, int line_num, string attr, vector<Type *> *vec) : value(
            value), type(type), line_num(line_num), scope_num(scope_num) {
        if (attr == "VAR") {
            this->attribute = VARIABLE;
        } else if (attr == "FUNC") {
            this->attribute = FUNCTION;
        } else if (attr == "STRUCT") {
            this->attribute = STRUCTURE;
        } else if (attr == "EXP") {
            this->attribute = EXPRESSION;
        }
        for (int i = 0; i < vec->size(); i++) {
            args.push_back((*vec)[i]);
        }
    }

};


class SymbolTable {
public:
    vector <string> key_list;
    unordered_multimap<string, SymbolElement *> symbolTableInstance;
    int scope;

    bool insertEntry(string id, SymbolElement *entry) {

        if (this->symbolTableInstance.find(id) != this->symbolTableInstance.end()) {
            if (id.substr(0, 4) == "FUNC" || id.substr(0, 6) == "STRUCT") {
                return false;
            }
            auto range = this->symbolTableInstance.equal_range(id);
            for (auto it = range.first; it != range.second; ++it) {
                if (it->second->scope_num == this->scope) {
                    return false;
                }
            }
        }
        D(cerr << "succefully insert " << id << endl;)
        this->symbolTableInstance.emplace(id, entry);

        return true;
    }

    SymbolElement *searchEntry(string name, string type) {
        string id = type + "_" + name;
        auto item = this->symbolTableInstance.find(id);

        if (item == symbolTableInstance.end()) {
            return nullptr;
        } else {
            return item->second;
        }
    }

    void showTable() {
        auto it = this->symbolTableInstance.begin();
        cout << "===========================" << endl;
        while (it != this->symbolTableInstance.end()) {
            cout << " ID : " << it->first << "   " << it->second->type->name << "scope : " << it->second->scope_num  << endl;
            it++;
        }
        cout << "===========================" << endl;
    }

    void flushTable() {
        auto it = this->symbolTableInstance.begin();
        while (it != this->symbolTableInstance.end()) {
            if (it->second->scope_num == this->scope && it->first.substr(0,4) != "FUNC") {
                this->symbolTableInstance.erase(it++);
            } else {
                it++;
            }
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
void semanticEntry(ast_node *);

void extDefListEntry(ast_node *);

void extDefEntry(ast_node *);

Type *specifierEntry(ast_node *);

Type *structSpecifierEntry(ast_node *);

Type *varDecEntry(ast_node *, Type *);

bool funDecEntry(ast_node *, Type *);

Type *typeEntry(ast_node *);

vector<Type *> varListEntry(ast_node *);

Type *paramDecEntry(ast_node *);

Type *varListLoopEntry(ast_node *);

void compStEntry(ast_node *, string);

pair<string, SymbolElement *> createTableInfo(ast_node *, Type *, string, vector<Type *> *);

void defEntry(ast_node *node, vector<Type *> *vec_po);

void defListEntry(ast_node *node, vector<Type *> *vec_po);

void decListEntry(ast_node *node, Type *, vector<Type *> *vec_po);

void decEntry(ast_node *node, Type *, vector<Type *> *vec_po);

Type *ExpressionEntry(ast_node *);

void stmtListEntry(ast_node *, string);

void stmtEntry(ast_node *node, string);

void argsEntry(ast_node *node, vector<Type *> *args);

void extDecList(ast_node *node, Type *type);

void reportError(int type, int line_num, string diy) {
    cout << "Error type " << type << " at Line " << line_num << ": " << error_info[type] << diy << endl;
}

bool equalType(Type* left, Type* right){
    if(left->name == right->name){
        return true;
    }else{
        return false;
    }
}

Type* createEmptyType(int lVal){
    Type * empty = new Type();
    empty->name = "INVALID";
    empty->lVal = lVal;
    return empty;
}

Type *copyType(Type *type) {
    Type *res;
    if (type->name.substr(0, 9) == "Primitive") {
        res = new PrimitiveType(type->name.substr(10));
    } else if (type->name.substr(0, 9) == "Structure") {
        StructureType *tmp_st_type = dynamic_cast<StructureType *>(type);
        vector < Type * > *vec = new vector<Type *>;
        for (int i = 0; i < tmp_st_type->field_size; i++) {
            vec->push_back(copyType(tmp_st_type->fields[i]));
        }
        res = new StructureType(vec);
    } else if (type->name.substr(0, 5) == "Array") {
        ArrayType *tmp_array_type = dynamic_cast<ArrayType *>(type);
        res = new ArrayType(copyType(tmp_array_type->base), tmp_array_type->size);
    }
    res->name = type->name;
    res->filed_name = type->filed_name;
    res->lVal = type->lVal;
    return res;
}

void semanticEntry(ast_node *root) {
    if (root->children_num <= 0)return;
    D(cerr << ">> root children size : " << root->children.size() << endl;)
    D(cerr << "lineno: " << __LINE__ << " " << root->printNode() << endl;)
    extDefListEntry(root->children[0]);
}


void extDefListEntry(ast_node *node) {
    if (node->children_num <= 0) return; // some leaf nodes have no children, which will lead to break down
    extDefEntry(node->children[0]);
    if (node->children_num == 1) return;
    else {
        extDefListEntry(node->children[1]);
        return;
    }
}

void extDefEntry(ast_node *node) {
    if (node->children_num <= 0)return;
    Type *spec_type = specifierEntry(node->children[0]);
    if (node->children_num >= 2) {
        ast_node *child = node->children[1];
        if (child->name == "FunDec" && node->children_num == 3) {
            bool access = funDecEntry(child, spec_type);
            if (child->children_num > 2 && child->children[2]->name == "VarList") {
                symbolTable.scope--;
            }
            if (access)compStEntry(node->children[2], child->children[0]->value);
        } else if (child->name == "ExtDecList") {
            extDecList(child, spec_type);
        }
    }
}

void extDecList(ast_node *node, Type *type) {
    varDecEntry(node->children[0], type);
    if (node->children_num > 2) {
        extDecList(node->children[2], type);
    }
}

void compStEntry(ast_node *node, string func_id) {
    symbolTable.scope++;
    if (node->children_num != 4)return;
    defListEntry(node->children[1], nullptr);
    stmtListEntry(node->children[2], func_id);
    symbolTable.flushTable();
}

void defListEntry(ast_node *node, vector<Type *> *vec) {
    if (node->children_num <= 0) return;
    defEntry(node->children[0], vec);
    if (node->children_num >= 2) {
        defListEntry(node->children[1], vec);
    }
}

void stmtListEntry(ast_node *node, string func_id) {
    if (node->children_num <= 0) return;
    stmtEntry(node->children[0], func_id);
    if (node->children_num >= 2) {
        stmtListEntry(node->children[1], func_id);
    }
}

void stmtEntry(ast_node *node, string func_id) {
    if (node->children_num <= 0) return;
    if (node->children_num == 2 && node->children[0]->name == "Exp") {
        ExpressionEntry(node->children[0]);
    } else if (node->children_num == 1 && node->children[0]->name == "CompSt") {
        compStEntry(node->children[0], func_id);
    } else if (node->children_num == 3 && node->children[0]->name == "RETURN") {
        D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
        Type *expType = ExpressionEntry(node->children[1]);
        if (expType->name != "INVALID") {
            SymbolElement *info = symbolTable.searchEntry(func_id, "FUNC");
            if (info->type->name == "INVALID" || info->type->name != expType->name) {
                reportError(8, node->line_num, "");
            }
        }
    } else if (node->children[0]->name == "IF") {
        Type *if_type_exp = ExpressionEntry(node->children[2]);
        stmtEntry(node->children[4], func_id);
        if (node->children_num > 6) {
            stmtEntry(node->children[6], func_id);
        }
    } else if (node->children_num == 5 && node->children[0]->name == "WHILE") {
        Type *type = ExpressionEntry(node->children[2]);
        stmtEntry(node->children[4], func_id);
    }
}

void defEntry(ast_node *node, vector<Type *> *vec_po) {
    if (node->children_num != 3)return;
    Type *spec_type = specifierEntry(node->children[0]);
    decListEntry(node->children[1], spec_type, vec_po);
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
}

void decListEntry(ast_node *node, Type *spec_type, vector<Type *> *vec_po) {
    if (node->children_num <= 0)return;
    decEntry(node->children[0], spec_type, vec_po);
    if (node->children_num == 3) {
        decListEntry(node->children[2], spec_type, vec_po);
    }
}

void decEntry(ast_node *node, Type *spec_type, vector<Type *> *vec_po) {
    if (node->children_num <= 0)return;
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    Type *type = varDecEntry(node->children[0], spec_type);
    if (vec_po != nullptr) {
        vec_po->push_back(type);
    }
    if (node->children_num == 3) {
        Type *type_exp = ExpressionEntry(node->children[2]);
        if (type->name != "INVALID" && type_exp->name != "INVALID" && type_exp->name != type->name) {
            reportError(5, node->line_num, "");
        }
    }

}

Type *checkFunc(ast_node *node, vector<Type *> *args) {
    SymbolElement *info_FUNC = symbolTable.searchEntry(node->value, "FUNC");
    SymbolElement *info_VAR = symbolTable.searchEntry(node->value, "VAR");
    if (info_FUNC == nullptr && info_VAR != nullptr) {
        reportError(11, node->line_num, "" + node->value);
        return createEmptyType(0);
    } else if (info_FUNC == nullptr && info_VAR == nullptr) {
        reportError(2, node->line_num, " " + node->value);
        return createEmptyType(0);
    }
    if (args != nullptr) {
        if (info_FUNC->args.size() != args->size()) {
            reportError(9, node->line_num,
                        "argument number for compare, expect " + to_string(info_FUNC->args.size()) + ", got " + to_string(args->size()));
        }else{
            for(int i = 0; i < args->size() ; i++){
                if(!equalType((*args)[i],info_FUNC->args[i])){
                    reportError(9, node->line_num,
                                "argument type for compare in function "+node->value);
                    break;
                }
            }
        }
    }
    return info_FUNC->type;
}

Type *ExpressionEntry(ast_node *node) {
    if (node->children_num <= 0)return createEmptyType(0);
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    if (node->children_num == 1 && node->children[0]->name == "ID") {
        SymbolElement *po = symbolTable.searchEntry(node->children[0]->value, "VAR");
        if (po == nullptr) {
            reportError(1, node->children[0]->line_num, " " + node->children[0]->value);;
            return createEmptyType(1);;
        }
        po->type->lVal = 1;
        return po->type;
    } else if (node->children[0]->name == "ID" && node->children_num >= 3) {
        vector < Type * > *args = new vector<Type *>;
        if (node->children[2]->name == "Args") {
            argsEntry(node->children[2], args);
        }
        return checkFunc(node->children[0], args);
    } else if (node->children_num == 1 && (node->children[0]->name == "INT" || node->children[0]->name == "FLOAT" ||
                                           node->children[0]->name == "CHAR")) {
        string str_type = node->children[0]->name;
        transform(str_type.begin(), str_type.end(), str_type.begin(), ::tolower);
        Type *type = new PrimitiveType(str_type);
        return type;
    } else if (node->children[0]->name == "Exp") {
        D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
        Type *left_exp_type = ExpressionEntry(node->children[0]);
        if (node->children_num == 3 && node->children[1]->name == "ASSIGN") {
            Type *right_exp_type = ExpressionEntry(node->children[2]);
            if(left_exp_type->lVal == 0){
                reportError(6, node->children[0]->line_num, "");
            }
            int invalid_cnt = 0;
            if(left_exp_type->name == "INVALID") invalid_cnt++;
            if(right_exp_type->name == "INVALID") invalid_cnt++;
            if(invalid_cnt >= 1){
                if(invalid_cnt == 1){
                    reportError(5, node->line_num, "");
                }
                return createEmptyType(0);
            }

            if (right_exp_type->name != "INVALID" && left_exp_type->name != "INVALID") {
                string co_array = "Array";
                string namel = left_exp_type->name;
                string namer = right_exp_type->name;
                if(namel.compare(0, co_array.size(), co_array) == 0){
                    namel = namel.substr(6);
                }
                if(namer.compare(0, co_array.size(), co_array) == 0){
                    namer = namer.substr(6);
                }
                if (namel != namer) {
                    reportError(5, node->line_num, "");
                    return createEmptyType(0);
                }
            }
            return left_exp_type;
        } else if (node->children_num == 3 && (node->children[1]->name == "PLUS"
                                               || node->children[1]->name == "MINUS"
                                               || node->children[1]->name == "MUL"
                                               || node->children[1]->name == "DIV"
                                               || node->children[1]->name == "LT"
                                               || node->children[1]->name == "LE"
                                               || node->children[1]->name == "GT"
                                               || node->children[1]->name == "GE"
                                               || node->children[1]->name == "NE"
                                               || node->children[1]->name == "EQ")) {
            Type *right_exp_type = ExpressionEntry(node->children[2]);

            /*
             * based on the Assumption 3 only int and float variables can do arithmetic operations
             * */

            if (left_exp_type->name != "INVALID" && right_exp_type->name != "INVALID") {
                if ((left_exp_type->name == "Primitive_float" || left_exp_type->name == "Primitive_int")
                    && (right_exp_type->name == "Primitive_float" || right_exp_type->name == "Primitive_int")
                        ) {
                    if ((node->children[1]->name == "PLUS"
                         || node->children[1]->name == "MINUS"
                         || node->children[1]->name == "MUL"
                         || node->children[1]->name == "DIV")) {
                        if (left_exp_type->name == "Primitive_float" || right_exp_type->name == "Primitive_float") {
                            return new PrimitiveType("float");
                        } else {
                            return new PrimitiveType("int");
                        }
                    }
                    left_exp_type->lVal = (left_exp_type->lVal || right_exp_type->lVal);
                    return left_exp_type;
                } else {
                    reportError(7, node->line_num, "");
                    return createEmptyType(0);
                }
            } else {
                return createEmptyType(0);
            }
        } else if (node->children[1]->name == "AND" || node->children[1]->name == "OR") {
            Type *right_exp_type = ExpressionEntry(node->children[2]);
            if (left_exp_type->name != "INVALID" && right_exp_type->name != "INVALID") {
                if (left_exp_type->name != "Primitive_int" || right_exp_type->name != "Primitive_int") {
                    reportError(16, node->line_num, "");
                    return createEmptyType(0);
                } else {
                    return new PrimitiveType("int");
                }
            } else {
                return createEmptyType(0);
            }
        } else if (node->children_num == 3 && node->children[1]->name == "DOT") {
            if (left_exp_type->name != "INVALID") {
                string co_array = "Array";
                StructureType *stType;
                string lname = left_exp_type->name;
                while(lname.compare(0, co_array.size(), co_array) == 0){
                    lname = lname.substr(6);
                }
                if (lname != "Structure") {
                    reportError(13, node->line_num, "");
                    return createEmptyType(1);
                }
                lname = left_exp_type->name;
                ArrayType *tmp_array_po;
                Type* tmp_type = left_exp_type;
                while(lname.compare(0, co_array.size(), co_array) == 0){
                    lname = lname.substr(6);
                    tmp_array_po = dynamic_cast<ArrayType* >(tmp_type);
                    tmp_type = tmp_array_po->base;
                }
                stType = dynamic_cast<StructureType *>(tmp_type);
                Type *rt_type = nullptr;
                for (int i = 0; i < stType->field_size; i++) {
                    if (stType->fields[i]->filed_name == node->children[2]->value) {
                        rt_type = stType->fields[i];
                        break;
                    }
                }
                if (rt_type == nullptr) {
                    reportError(14, node->line_num, ": " + node->children[2]->value);
                    return createEmptyType(1);
                }
                rt_type->lVal = 1;
                return rt_type;
            }
            return createEmptyType(1);
        } else if (node->children_num == 4 && node->children[1]->name == "LB") {
            int isError = 0;
            if( left_exp_type->name != "INVALID" && dynamic_cast<ArrayType*>(left_exp_type) == nullptr){
                reportError(10, node->line_num, "");
                isError = 1;
            }
            Type *right_exp_type = ExpressionEntry(node->children[2]);
            if (right_exp_type->name != "Primitive_int") {
                reportError(12, node->line_num, "");
                isError = 1;
            }
            if(right_exp_type->name == "INVALID" ||  left_exp_type->name == "INVALID" || isError){
                return createEmptyType(1);
            }
            Type* inside_array = dynamic_cast<ArrayType*>(left_exp_type)->base;
            inside_array->lVal = 1;
            return inside_array;
        }
    } else if (node->children_num == 2 && node->children[0]->name == "MINUS") {
        return ExpressionEntry(node->children[1]);
    } else if(node->children[0]->name == "NOT"){
        return ExpressionEntry(node->children[1]);
    } else if(node->children_num == 3 && node->children[0]->name == "LP"){
        return ExpressionEntry(node->children[1]);
    }
    return createEmptyType(0);
}

Type *specifierEntry(ast_node *node) {
    if (node->children_num != 1) return createEmptyType(1); //Specifier only have one child
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    if (node->children[0]->name == "TYPE") {
        Type *spec_type = typeEntry(node->children[0]);
        return spec_type;
    } else if (node->children[0]->name == "StructSpecifier") {
        Type *spec_type = structSpecifierEntry(node->children[0]);
        return spec_type;
    }
    return createEmptyType(1);
}

void argsEntry(ast_node *node, vector<Type *> *args) {
    args->push_back(ExpressionEntry(node->children[0]));
    if (node->children_num > 2) {
        argsEntry(node->children[2], args);
    }
}

Type *typeEntry(ast_node *node) {
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    Type *res = new PrimitiveType(node->value);
    string name = "Primitive_" + node->value;
    return res;
}

Type *structSpecifierEntry(ast_node *node) {
    if (node->children_num <= 0) return createEmptyType(1);
    if (node->children_num == 5) {
        vector < Type * > *vec = new vector<Type *>;
        defListEntry(node->children[3], vec);
        Type *structType = new StructureType(vec);
        D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
        SymbolElement *element = new SymbolElement(node->value, structType, symbolTable.scope, node->line_num,
                                                   "STRUCT");
        string id = "STRUCT_" + node->children[1]->value;
        if (!symbolTable.insertEntry(id, element)) {
            reportError(15, node->children[0]->line_num, ": " + node->children[1]->value);
            return createEmptyType(1);
        }
        return structType;
    } else if (node->children_num == 2 && node->children[1]->name == "ID") {
        D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
        SymbolElement *info = symbolTable.searchEntry(node->children[1]->value, "STRUCT");
        if (info != nullptr) {
            return info->type;
        } else {
            reportError(14, node->line_num, "");
        }
    }
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    return createEmptyType(1);
}

bool funDecEntry(ast_node *node, Type *spec_type) {
    if (node->children_num <= 2) return false;
    if (node->children[2]->name == "VarList") {
        symbolTable.scope++;
        vector < Type * > parm_types = varListEntry(node->children[2]);
        pair < string, SymbolElement * > info = createTableInfo(node->children[0], spec_type, "FUNC", &parm_types);
        if (!symbolTable.insertEntry(info.first, info.second)) {
            reportError(4, node->children[0]->line_num, " " + node->children[0]->value);
            return false;
        }
        D(cerr << "lineno: " << __LINE__ << " parm_types size: " << parm_types.size() << endl;)
    } else {
        pair < string, SymbolElement * > info = createTableInfo(node->children[0], spec_type, "FUNC", nullptr);
        if (!symbolTable.insertEntry(info.first, info.second)) {
            reportError(4, node->children[0]->line_num, " " + node->children[0]->value);
            return false;
        }
    }
    return true;

}

vector<Type *> varListEntry(ast_node *node) {
    vector < Type * > res;
    if (node->children_num <= 0)return res;
    res.push_back(paramDecEntry(node->children[0]));
    ast_node *begin_node = node;
    while (begin_node->children_num >= 3) {
        begin_node = begin_node->children[2];
        res.push_back(paramDecEntry(begin_node->children[0]));
    }
    return res;
}

Type *paramDecEntry(ast_node *node) {
    if (node->children_num <= 0)return createEmptyType(0);
    Type *spec_type = specifierEntry(node->children[0]);
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    varDecEntry(node->children[1], spec_type);
    return spec_type;
}

Type *varDecEntry(ast_node *node, Type *spec_type) {
    if (node->children_num <= 0)return createEmptyType(1);
    D(cerr << "lineno: " << __LINE__ << " " << node->printNode() << endl;)
    if (node->children[0]->name == "ID") {
        if(spec_type->name == "INVALID"){
            return createEmptyType(1);
        }
        spec_type->filed_name = node->children[0]->value;
        Type *res = copyType(spec_type);
        pair < string, SymbolElement * > info = createTableInfo(node->children[0], spec_type, "VAR", nullptr);
        if (!symbolTable.insertEntry(info.first, info.second)) {
            reportError(3, node->children[0]->line_num, " " + node->children[0]->value);
            return createEmptyType(1);
        }
        return res;
    } else if (node->children[0]->name == "VarDec" && node->children_num == 4) {
        Type *array_type = new ArrayType(spec_type, atoi(node->children[2]->value.c_str()));
        D(cerr << "lineno: " << __LINE__ << " " << node->children_num << endl;)
        return varDecEntry(node->children[0], array_type);
    }
    return createEmptyType(1);

}

pair<string, SymbolElement *> createTableInfo(ast_node *node, Type *type, string attr, vector<Type *> *vec) {
    D(cerr << "lineno: " << __LINE__ << " node name : " << node->name << endl;)
    if (attr == "VAR") {
        D(cerr << "lineno: " << __LINE__ << " node value : " << node->value << endl;)
        type->lVal = 1;
        SymbolElement *element = new SymbolElement(node->value, type, symbolTable.scope, node->line_num, "VAR");
        string id = "VAR_" + node->value;
        D(cerr << "element info: " << " entry ID: " << id << endl;)
        return make_pair(id, element);
    } else if (attr == "FUNC") {
        //PrimitiveType* primitiveType = dynamic_cast<PrimitiveType*>(type);
        D(cerr << "lineno: " << __LINE__ << " node value : " << node->value << endl;)
        if (vec != nullptr) {
            SymbolElement *element = new SymbolElement(node->value, type, symbolTable.scope, node->line_num, "FUNC",
                                                       vec);
            string id = "FUNC_" + node->value;
            return make_pair(id, element);
        } else {
            SymbolElement *element = new SymbolElement(node->value, type, symbolTable.scope, node->line_num, "FUNC");
            string id = "FUNC_" + node->value;
            return make_pair(id, element);
        }
    }
    return make_pair("", nullptr);
}

#endif
