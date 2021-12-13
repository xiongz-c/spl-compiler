# CS323 Project2 - Report

```
Yifei Li 11811905
Zhuochen Xiong 11811806
```

# Introduction

In project2, our task is to perform semantic detection on the given SPL code statements. Among other things, the basic task is to complete the type checking, with an additional bonus for completing the scope detection. In this project, the given test samples are guaranteed to be syntactically correct, and our program can detect semantic errors in syntactically correct programs.



# Design & Implementation

### file structure

```
.
├── ast.hpp
├── bin
│   └── splc
├── lex.l
├── Makefile
├── semantic.hpp
├── syntax.y
```

### Symbol Table

```cpp
class SymbolTable{
public:
    vector<string> key_list;
    unordered_multimap<string,SymbolElement*> symbolTableInstance;
    int scope; // define scope depth to identify scope
				
  	bool insertEntry(string id, SymbolElement* entry);
    SymbolElement* searchEntry(string name, string type);
    void flushTable();

};
```

In the design of Symbol Table, we use the unordered_multimap of C++ STL as the main data structure. Where key is the variable, function and user-defined variable type, and value is the information corresponding to the symbol. The information is mainly the scope information for scope detection, the return type and parameter list of the method, and the content of the user-defined structure. The unordered_multimap can store multiple values with the same key.

### Type Representation

```cpp
class Type{
public:
    string name;
    string filed_name;
    int lVal;
    void setName(string s){
        this->name = s;
    }
    virtual ~Type() = default;
};

class PrimitiveType: public Type{...};
class ArrayType: public Type{...};

class StructureType: public Type{...};

```

Type checking is an important part of semantic detection. In SPL, two types are defined: primitive  types and derived types.In SPL, two types are defined: basic data types and derived types.  We need to perform type checking on both sides of assignment statements and at function invocation.

### Semantic Analysis

#### Rules

```
Type 1 a variable is used without a definition
Type 2 a function is invoked without a definition
Type 3 a variable is redefined in the same scope
Type 4 a function is redefined (in the global scope, since we don’t have nested functions)
Type 5 unmatching types appear at both sides of the assignment operator (=)
Type 6 rvalue appears on the left-hand side of the assignment operator
Type 7 unmatching operands, such as adding an integer to a structure variable
Type 8 a function’s return value type mismatches the declared type
Type 9 a function’s arguments mismatch the declared parameters (either types or num-
bers, or both)
Type 10 applying indexing operator ([...]) on non-array type variables
Type 11 applying function invocation operator (foo(...)) on non-function names
Type 12 array indexing with a non-integer type expression
Type 13 accessing members of a non-structure variable (i.e., misuse the dot operator)
Type 14 accessing an undefined structure member
Type 15 redefine the same structure type
```



#### Analysis Methodology

```cpp
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
Type* varDecEntry(ast_node*, Type*);
bool funDecEntry(ast_node*, Type*);
Type* typeEntry(ast_node*);
vector<Type*> varListEntry(ast_node*);
Type* paramDecEntry(ast_node*);
Type* varListLoopEntry(ast_node*);
void compStEntry(ast_node*,string);
pair<string, SymbolElement*> createTableInfo(ast_node*,  Type*, string, vector<Type*>*);
void defEntry(ast_node* node,vector<Type*> * vec_po);
void defListEntry(ast_node* node,vector<Type*> * vec_po);
void decListEntry(ast_node* node, Type*,vector<Type*> * vec_po);
void decEntry(ast_node* node,Type*,vector<Type*> * vec_po);
Type* ExpressionEntry(ast_node*);
void stmtListEntry(ast_node*,string);
void stmtEntry(ast_node* node,string);
void argsEntry(ast_node* node, vector<Type*>* args);
```

Follow the language definition, each node of the abstract syntax tree should be checked. So each of their nodes corresponds to a function. Type informtion should be passed among several functions.

### Optional Feature (Bonus)

* **Scope checking**

```cpp
class SymbolTable{
public:
    vector<string> key_list;
    unordered_multimap<string,SymbolElement*> symbolTableInstance;
    int scope; // define scope depth to identify scope

		...
};
```

Record the current scope depth by defining the scope property in the *SymbolTable*.  When entering a comparison statement or a new function, we increase the value of scope and conversely decrease the value of scope, thus having the effect of recording the scope of the current variable.

```cpp
void flushTable(){
    auto it = this->symbolTableInstance.begin();
    while(it!= this->symbolTableInstance.end()){
        if(it->second->scope_num == this->scope){
            this->symbolTableInstance.erase(it++);
        }else{
            it++;
        }
    }
}
```

In the *SymbolTable* Class, the *flushTable* method is defined to clear all variables under the current scope.

Here is a example.

```
int v;
int a[2];

struct test{
    float m1;
}g1;

float test_1() {
    v = 1;
    if(v > 0) {
        char v = 'a';
    }
    v = a[0] + 2;
    return v+g1.m1;
}
```

In this testcase, there is no error. Our semantic analysis can recognize the different 'v' in different scopes. In the following testcase, it can represent the type information of each 'v'.

```
int v;
int a[2];

struct test{
    float m1;
}g1;

float test_1() {
    v = 2.0;  //global scope v is int, error!
    v = 1;
    if(v > 0) {
        char v = 'a';
        v = 2.0;  // in current scope, v is char, error!
    }
    v = a[0] + 2;
    return v+g1.m1;
}


Error type 5 at Line 9: unmatching type on both sides of assignment
Error type 5 at Line 13: unmatching type on both sides of assignment
```

