#include "semantic.hpp"

#define ARG1 0
#define ARG2 1
#define RESULT 2
using namespace std;


int tmp_cnt, var_cnt, label_cnt;

string Tmp(){return "t" + to_string(++tmp_cnt);}
string Var(){return "v" + to_string(++var_cnt);}
string Label(){return "label" + to_string(++label_cnt);}
string remove_tmp(){tmp_cnt--}
float str_to_num(string value, bool type){
    if (!type) // DEFAULT: parse to int when false
        atoi(value.c_str());
    else // else parse to float
        atof(value.c_str());
}
string val2str(int num) {
    if (num > 0) {
        return "t" + to_string(num);
    } else {
        return "#" + to_string(-num);
    }
}


class Tac {
public:
    string op;
    string operands[3]; // {ARG1,ARG2,RESULT}
    vector<int> suffix; // suffix mem
    vector<int> sizes; //
    bool swap_flag = false;
    Type *type;
    enum TacType{
        LABEL, FUNC,
        ASSIGN,
        ARITH,
        ASSIGN_ADDRESS, ASSIGN_VALUE, COPY_VALUE, CALL
        IF,
        DEC,
        GOTO, RETURN, PARAM, ARG, READ, WRITE
        EXIT} tac_type;
    Tac(TacType tac_type) {
        this->tac_type = tac_type;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& op, const string& arg1) {
        this->tac_type = tac_type;
        this->op = op;
        this->operands[ARG1] = arg1;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& op, const string& arg1, const string& res) {
        this->tac_type = tac_type;
        this->op = op;
        this->operands[ARG1] = arg1;
        this->operands[RESULT] = res;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& op, const string& arg1, const string& arg2, const string& res) {
        this->tac_type = tac_type;
        this->op = op;
        this->operands[ARG1] = arg1;
        this->operands[ARG2] = arg2;
        this->operands[RESULT] = res;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& res, vector<int> arr) {
        this->tac_type = tac_type;
        this->op = "DEC"; // just for dec
        this->operands[RESULT] = res;

        int tot = 1;
        for(int i = arr.size() - 1; i >= 0; --i) {
            this->suffix.push_back(tot);
            tot *= arr[i];
        }
        // todo 不确定需不需要size vector, 先不操作
        this->operands[ARG1] = std::to_string(tot);
        swap_flag = false;
    }

    string to_string() {
        switch (tac_type) {
            case ASSIGN:
                printf("%s := %s\n", operands[RESULT].c_str(), operands[ARG1].c_str());
                break;
            case ARITH: // op: [relop]
                printf("%s := %s %s %s\n", operands[RESULT].c_str(), operands[ARG1].c_str(), op.c_str(), operands[ARG2].c_str());
                break;
            case IF: // op: [relop]
                printf("IF %s %s %s GOTO %s\n", operands[ARG1].c_str(), op.c_str(), operands[ARG2].c_str(), operands[RESULT].c_str());
                break;
            case DEC: // op: DEC
                printf("%s %s %s\n", op.c_str(), operands[RESULT].c_str(), operands[ARG1].c_str());
                break;
            case ASSIGN_ADDRESS:// op: &
            case ASSIGN_VALUE:// op: *
            case CALL: // op: CALL
                printf("%s := %s %s\n", operands[RESULT].c_str(), op.c_str(), operands[ARG1].c_str());
                break;
            case COPY_VALUE: // op: *
                printf("%s%s := %s \n", op.c_str(), operands[RESULT].c_str(), operands[ARG1].c_str());
                break;
            case FUNC: // op: FUNCTION
            case LABEL: // op: LABEL
                printf("%s %s :\n", op.c_str(), operands[ARG1].c_str());
                break;
            case EXIT: // last line. maybe for ir optimize
                printf("\n");
                break;
            default:// op: GOTO/RETURN/ARG/PARAM/READ/WRITE
                printf("%s %s\n", op.c_str(), operands[ARG1].c_str());
        }
    }

    virtual int append_self() {
        tac_vector.push_back(tac);
        return tac_vector.size() - 1;
    }

};

vector<Tac*> tac_vector;
unordered_map<string, int> ir_table; //

int append_tac(Tac *tac) {
    tac_vector.push_back(tac);
    return tac_vector.size() - 1;
}

void ir_generate() {
    for (auto itr = tac_vector.begin(); itr != tac_vector.end(); ++itr) {
        (*itr)->to_string();
    }
}

void put_ir(string name, int id){ir_table[name] = id;}
int get_ir(string name){return ir_table[name];}

void ir_init() {
    tac_vector.clear();
    ir_table.clear();
}

/**
 * recursive function declare [Analyze Semantic Entry]
 */
void ir_starter(ast_node *root);
void ir_ext_def_list(ast_node *node);
void ir_ext_dec_list(ast_node *node, Type * type);
void ir_ext_def(ast_node *node);
Type *ir_specifier(ast_node *node);
Type *ir_type(ast_node *node);
Type *ir_struct_specifier(ast_node *node);
void ir_func(ast_node *node, Type *type);
void ir_comp_stmt(ast_node *node);
void ir_def_list(ast_node *node);
void ir_def(ast_node *node);
void ir_dec_list(ast_node *node, Type *type);
void ir_stmt(ast_node *node);
void ir_stmt_list(ast_node *node);
void ir_dec(ast_node *node, Type *type);
Tac* ir_var_dec(ast_node *node, Type* type);
int ir_exp(ast_node *node, bool single=false);
void ir_var_list(ast_node *node);
void ir_param_dec(ast_node *node);
vector<int> ir_args(ast_node *node);

/**
 * Program: ExtDefList
*/
void ir_starter(ast_node *root) {
    ir_init();
    ir_ext_def_list(root->children[0]);
    ir_generate();
}

/**
 * ExtDefList: ExtDef ExtDefList | %empty
*/
void ir_ext_def_list(ast_node *node){
    while(node->children_num){
        ir_ext_def(node->children[0]);
        node = node->children[1];
    }
}

/**
 * ExtDef: Specifier ExtDecList SEMI
 *       | Specifier SEMI
 *       | Specifier FunDec CompSt
*/
void ir_ext_def(ast_node *node){
    Type *type = ir_specifier(node->children[0]);
    if(node->children[1]->name.compare("ExtDecList") == 0){
        ir_ext_dec_list(node->children[1], type);
    }
    if(node->children[1]->name.compare("FunDec") == 0){
        ir_func(node->children[1], type);
        ir_comp_stmt(node->children[2]);
    }
}

void ir_ext_dec_list(ast_node *node, Type *type){
    Tac *tac = ir_var_dec(node->children[0], type);
    while(node->children_num > 1){
        node = node->children[2];
        Tac *tac = ir_var_dec(node->children[0], type);
    }
    put_ir(tac->operands[RESULT], tac->append_self());
}

/**
 * Specifier: TYPE
 * Specifier: StructSpecifier
*/
Type *ir_specifier(ast_node *node){

}

Type *ir_type(ast_node *node){
    return typeEntry(node)
}

/**
 * StructSpecifier: STRUCT ID LC DefList RC
 * StructSpecifier: STRUCT ID
 */
Type *ir_struct_specifier(ast_node *node){

}

void ir_func(ast_node *node, Type *type){

}

/**
 * CompSt: LC DefList StmtList RC
*/
void ir_comp_stmt(ast_node *node){

}

/**
 * DefList: Def DefList
 * DefList: %empty
 */
void ir_def_list(ast_node *node){

}

/**
 * Specifier DecList SEMI
 */
void ir_def(ast_node *node){

}

/**
 * DecList: Dec | Dec COMMA DecList
*/
void ir_dec_list(ast_node *node, Type *type){

}

/**
 * StmtList: Stmt StmtList
 * StmtList: %empty
 */
void ir_stmt_list(ast_node *node){

}

/**
 * Stmt: Exp SEMI
 * Stmt: CompSt
 * Stmt: RETURN Exp SEMI
 * Stmt: IF LP Exp RP Stmt
 * Stmt: IF LP Exp RP Stmt ELSE Stmt
 * Stmt: WHILE LP Exp RP Stmt
 * WRITE LP Exp RP SEMI
 */
void ir_stmt(ast_node *node){

}

/**
 * Dec: VarDec
 * Dec: VarDec ASSIGN Exp
 */
void ir_dec(ast_node *node, Type *type){

}

/**
 * VarDec: ID
 * VarDec: VarDec LB INT RB
 */
Tac* ir_var_dec(ast_node *node, Type* type){
    vector<ast_node*> ast_vector;
    ast_vector.push_back(node);
    vector<int> int_vector;
    while (!ast_vector.empty()) {
        ast_node *now = ast_vector.back();
        if (now->children[0]->name == "ID") { // dec var
            ast_vector.pop_back();
            while (!ast_vector.empty()) {
                ast_node *nn = ast_vector.back();
                ast_vector.pop_back();
                int siz = str_to_num(nn->children[2]->value, nn->children[2]->name.compare("INT"));
                int_vector.push_back(siz);
            }
        }else{ // dec array
            ast_vector.push_back(now->children[0]);
        }
    }

    if (int_vector.size()) { // array
        return new Tac(Tac::DEC, Var(), int_vector);
    } else if (equalType(type, new StructureType())) { // structure
        return new Tac(Tac::DEC, Var(), {});
    } else {
        return new Tac(Tac::ASSIGN,"ASSIGN", val2str(0).c_str(), Var());
    }
}

/**
 * Exp: Exp ASSIGN Exp
 *    | Exp [{AND}|{OR}] Exp
 *    | Exp [{LT}|{LE}|{GT}|{GE}|{NE}|{EQ}] Exp
 *    | Exp [{PLUS}|{MINUS}|{MUL}|{DIV}] Exp
 *    | LP Exp RP
 *    | MINUS Exp | MINUS Exp %prec UMINUS
 *    | NOT Exp
 *    | ID LP Args RP
 *    | ID LP RP
 *    | ID
 *    | Exp LB Exp RB
 *    | Exp DOT ID
 *    | INT | FLOAT | CHAR
 *    | READ LP RP
 */
int ir_exp(ast_node *node, bool single=false){

}

/**
 * VarList: ParamDec COMMA VarList
 * VarList: ParamDec
 */
void ir_var_list(ast_node *node){

}

/**
 * ParamDec: Specifier VarDec
 */
void ir_param_dec(ast_node *node){

}

/**
 * Args: Exp COMMA Args
 * Args: Exp
 */
vector<int> ir_args(ast_node *node){

}


