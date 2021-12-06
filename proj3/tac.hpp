#include "semantic.hpp"

#define ARG1 0
#define ARG2 1
#define RESULT 2
using namespace std;


int tmp_cnt, var_cnt, label_cnt;

string Tmp(){return "t" + to_string(++tmp_cnt);}
string Var(){return "v" + to_string(++var_cnt);}
string Label(){return "label" + to_string(++label_cnt);}
int *genlist(int id){
    int *label = new int(id);
    return label;
}
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

    int append_self();
};

vector<Tac*> tac_vector;
unordered_map<string, int> ir_table;
vector<int> cont, br;

int Tac::append_self() {
    tac_vector.push_back(this);
    return tac_vector.size() - 1;
}

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
    cont.clear();
    br.clear();
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
void translate_cond_exp(ast_node *exp, string lb_t, string lb_f);
void translate_exp(ast_node *exp, string& place);

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
 * StructSpecifier: STRUCT ID LC DefList RC
 * StructSpecifier: STRUCT ID
*/
Type *ir_specifier(ast_node *node){
    Type *type;
    if(node->children[0]->name.compare("TYPE") == 0){
        type = typeEntry(node->children[0]);
    }else{
        type = structSpecifierEntry(node->children[0]);
    }
    return type;
}

void ir_func(ast_node *node, Type *type){
    string name = node->children[0]->value;
    int func_id = append_tac(new Tac(Tac::FUNC, "FUNCTION", name));
    put_ir(name, func_id);
    if (node->children[2]->name.compare("VarList") == 0) {
        ir_var_list(node->children[2]);
    }
}

/**
 * CompSt: LC DefList StmtList RC
*/
void ir_comp_stmt(ast_node *node){
    ir_def_list(node->children[1]);
    ir_stmt_list(node->children[2]);
}

/**
 * DefList: Def DefList
 * DefList: %empty
 */
void ir_def_list(ast_node *node){
    while(node->children_num){
        ir_def(node->children[0]);
        node = node->children[1];
    }
}

/**
 * Specifier DecList SEMI
 */
void ir_def(ast_node *node){
    Type *type = ir_specifier(node->children[0]);
    ir_dec_list(node->children[1], type);
}

/**
 * DecList: Dec | Dec COMMA DecList
*/
void ir_dec_list(ast_node *node, Type *type){
    ir_dec(node->children[0], type);
    while(node->children_num > 1){
        node = node->children[2];
        ir_dec(node->children[0], type);
    }
}

/**
 * StmtList: Stmt StmtList
 * StmtList: %empty
 */
void ir_stmt_list(ast_node *node){
    while(node->children_num){
        ir_stmt(node->children[0]);
        node = node->children[1];
    }
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
        // Exp SEMI
    if(node->children[0]->name.compare("Exp") == 0){
        ir_exp(node->children[0]);
    }
        // CompSt
    else if(node->children[0]->name.compare("CompSt") == 0){
        ir_comp_stmt(node->child[0]);
    }
        // RETURN Exp SEMI
    else if(node->children[0]->name.compare("RETURN") == 0){
        int exp_id = ir_exp(node->children[1]);
        append_tac(new Tac(Tac::RETURN, "RETURN", val2str(exp_id)));
    }
        // IF
    else if(node->children[0]->name.compare("IF") == 0){
        string lb1 = Label();
        string lb2 = Label();
        int exp_id = ir_exp(node->children[2]);
        int tbranch = append_tac(new Tac(Tac::LABEL, "LABEL", lb1));
        ir_stmt(node->children[4]);

        if(node->children_num < 6){
            int fbranch = append_tac(new Tac(Tac::GOTO, "GOTO", lb2));
            if_stmt(expid, tbranch, fbranch);
        }else {
            string lb3 = Label();
            jbranch = append_tac(new Tac(Tac::GOTO, "GOTO", lb3));
            append_tac(new Tac(Tac::LABEL, "LABEL", lb2));
            ir_stmt(node->child[6]);
            append_tac(new Tac(Tac::LABEL, "LABEL", lb3));
        }
    }
        // WHILE LP Exp RP Stmt
    else if(node->child[0]->type_name.compare("WHILE") == 0){
        string lb1 = Label();
        string lb2 = Label();
        string lb3 = Label();

        append_tac(new Tac(Tac::LABEL, "LABEL", lb1));


        append_tac(new Tac(Tac::LABEL, "LABEL", lb2));
        ir_stmt(node->children[4]);
        append_tac(new Tac(Tac::GOTO, "GOTO", lb1));
        append_tac(new Tac(Tac::LABEL, "LABEL", lb3));

    }
        // WRITE LP Exp RP SEMI
    else if (node->child[0]->type_name.compare("WRITE") == 0) {
        int id = irExp(node->child[2]);
        genid(new WriteTAC(tac_vector.size(), id));
    } else {
        assert(NULL);
    }
}

void translate_exp(ast_node *exp, string& place) {
    // todo 这个函数的作用是把place修改string的格式，看看如何替代
}

void translate_cond_exp(ast_node *exp, string lb_t, string lb_f){
    ast_node *child = exp->children[0];
    if (child->name == "EXP") {
        string sname = exp->children[1]->name;
        if (sname == "AND") {
            string lb1 = Label();
            translate_cond_exp(exp->children[0], lb1, lb_f);
            append_tac(new Tac(Tac::LABEL, "LABEL", lb1));
            translate_cond_exp(exp->children[2], lb_t, lb_f)
        } else if (sname == "OR") {
            string lb1 = Label();
            translate_cond_exp(exp->children[0], lb_t, lb_1);
            append_tac(new Tac(Tac::LABEL, "LABEL", lb1));
            translate_cond_exp(exp->children[2], lb_t, lb_f)
        } else {
            string t1 = Tmp();
            translate_exp(exp->children[2], t1);

            string t2 = Tmp();
            translate_exp(exp->children[2], t2);

            append_tac(new Tac(Tac::IF, exp->children[1]->value, t1, t2, lb_t));
            append_tac(new Tac(Tac::GOTO, "GOTO", lb_f));
        }
    } else if (child->name == "NOT") {
        translate_cond_Exp(exp->children[1], lb_f, lb_t);
    } else if (child->name == "LP") {
        translate_cond_exp(exp->children[1], lb_t, lb_f);
    }
}

/**
 * Dec: VarDec
 * Dec: VarDec ASSIGN Exp
 */
void ir_dec(ast_node *node, Type *type){
    int exp_id = 0;
    if(node->children_num > 1){
        exp_id = ir_exp(node->children[2]);
    }
    Tac *tac = ir_var_dec(node->children[0], type);
    if(exp_id){
        tac->operands[ARG1] = val2str(exp_id);
    }
    put_ir(tac->operands[RESULT], tac->append_self());
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
        return new Tac(Tac::DEC, Var(), vector<int>{});
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


