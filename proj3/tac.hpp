#include "semantic.hpp"
#include "list"
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
void remove_tmp(){tmp_cnt--;}
float str_to_num(string value, bool type){
    if (!type) // DEFAULT: parse to int when false
        return atoi(value.c_str());
    else // else parse to float
        return atof(value.c_str());
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
        ASSIGN_ADDRESS, ASSIGN_VALUE, COPY_VALUE, CALL,
        IF,
        DEC,
        GOTO, RETURN, PARAM, ARG, READ, WRITE,
        EXIT} tac_type;
    Tac(TacType tac_type) {
        this->tac_type = tac_type;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& op, const string& arg1) {
        this->tac_type = tac_type;
        this->op = op;
        this->operands[RESULT] = arg1;
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
    Tac(TacType tac_type, const string& op, const string& res, vector<int> arr, string name) {
        this->tac_type = tac_type;
        this->op = op; // just for dec or PARAM
        this->operands[RESULT] = res;
        this->operands[ARG2] = name; // 打印用不到，用于存var name

        int tot = 1;
        for(int i = arr.size() - 1; i >= 0; --i) {
            this->suffix.push_back(tot);
            tot *= arr[i];
        }
        // todo 不确定需不需要size vector, 先不操作
        this->operands[ARG1] = std::to_string(tot);
        swap_flag = false;
    }

    void to_string() {
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
                printf("%s %s\n", op.c_str(), operands[RESULT].c_str());
        }
    }

    string append_self();
};

vector<Tac*> tac_vector;
unordered_map<string, string> value_info; // key是变量名，value是变量编号
unordered_map<string, *Tac> type_info;// key 是变量编号，value是tac ——用变量编号就不用考虑scope了
vector<int> cont, br;

string Tac::append_self() {
    tac_vector.push_back(this);
    if (this->tac_type == Tac::FUNC) return std::to_string(tac_vector.size() - 1);
    return this->operands[RESULT];
}

string append_tac(Tac *tac) {
    tac_vector.push_back(tac);
    if (tac->tac_type == Tac::FUNC) return std::to_string(tac_vector.size() - 1);
    return tac->operands[RESULT];
}

void ir_generate() {
    for (auto itr = tac_vector.begin(); itr != tac_vector.end(); ++itr) {
        (*itr)->to_string();
    }
}

// 根据名字放入变量编号
void put_ir(string name, string vid){ir_table[name] = vid;}

// 根据名字取变量
string get_ir(string name){return ir_table[name];}

void check_refer(ast_node* exp, string& place) {
    if (exp->children[0]->name == "Exp" && exp->children.size() > 1) {
        ast_node* sibling = exp->children[1];
        if (sibling->name == "LB" || sibling->name == "DOT") {
            place = "*" + place;
        }
    }
}

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
void ir_func(ast_node *node);
void ir_comp_stmt(ast_node *node);
void ir_def_list(ast_node *node);
void ir_def(ast_node *node);
void ir_dec_list(ast_node *node, Type *type);
void ir_stmt(ast_node *node);
void ir_stmt_list(ast_node *node);
void ir_dec(ast_node *node, Type *type);
Tac* ir_var_dec(ast_node *node, Type* type);
void ir_var_list(ast_node *node);
void ir_param_dec(ast_node *node);
void translate_cond_exp(ast_node *exp, string lb_t, string lb_f);
void translate_exp(ast_node *exp, string& place);
void translate_args(ast_node *exp, list<string>* arg_list);

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
        ir_func(node->children[1]);
        ir_comp_stmt(node->children[2]);
    }
}

void ir_ext_dec_list(ast_node *node, Type *type){
    Tac *tac = ir_var_dec(node->children[0], type);
    while(node->children_num > 1){
        node = node->children[2];
        ir_ext_dec_list(node, type);
//        Tac *tac = ir_var_dec(node->children[0], type);
    }
    put_ir(tac->operands[ARG2], tac->append_self());
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

void ir_func(ast_node *node){
    string name = node->children[0]->value;
    string func_id = append_tac(new Tac(Tac::FUNC, "FUNCTION", name));
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
        string place;
        translate_exp(node->children[0], place);
    }
        // CompSt
    else if(node->children[0]->name.compare("CompSt") == 0){
        ir_comp_stmt(node->children[0]);
    }
        // RETURN Exp SEMI
    else if(node->children[0]->name.compare("RETURN") == 0){
        string tp = Tmp();
        translate_exp(node->children[1], tp);
        check_refer(node->children[1], tp);
        append_tac(new Tac(Tac::RETURN, "RETURN", tp));
    }
        // IF
    else if(node->children[0]->name.compare("IF") == 0){
        string lb1 = Label();
        string lb2 = Label();

        translate_cond_exp(node->children[2], lb1, lb2);
        append_tac(new Tac(Tac::LABEL, "LABEL", lb1));

        ir_stmt(node->children[4]);

        if(node->children_num < 6){
            append_tac(new Tac(Tac::LABEL, "LABEL", lb2));
        }else {
            string lb3 = Label();
            append_tac(new Tac(Tac::GOTO, "GOTO", lb3));
            append_tac(new Tac(Tac::LABEL, "LABEL", lb2));
            ir_stmt(node->children[6]);
            append_tac(new Tac(Tac::LABEL, "LABEL", lb3));
        }
    }
        // WHILE LP Exp RP Stmt
    else if(node->children[0]->name.compare("WHILE") == 0){
        string lb1 = Label();
        string lb2 = Label();
        string lb3 = Label();

        append_tac(new Tac(Tac::LABEL, "LABEL", lb1));
        translate_cond_exp(node->children[2], lb2, lb3);

        append_tac(new Tac(Tac::LABEL, "LABEL", lb2));
        ir_stmt(node->children[4]);
        append_tac(new Tac(Tac::GOTO, "GOTO", lb1));
        append_tac(new Tac(Tac::LABEL, "LABEL", lb3));

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
void translate_exp(ast_node *exp, string& place) {
    ast_node *child = exp->children[0];
    if (child->name == "INT") {
        place = "#" + child->value;
        remove_tmp();
    } else if (child->name == "ID") {
        if (exp->children.size() == 1) {
            place = get_ir(child->value);
            Type* expType = ExpressionEntry(exp);
            if (expType->is_refer) place = "&" + place;
            remove_tmp();
        } else {
            // function invoke
            if (place.empty()) place = Tmp();
            if (exp->children.size() == 3) { // ID LP RP
                if (child->value == "read") {
                    append_tac(new Tac(Tac::READ, "READ", place));
                }else {
                    append_tac(new Tac(Tac::CALL, "CALL", child->value, place));
                }
            } else { // ID LP ARGS RP
                if (child->value == "write") {
                    string tp = Tmp();
                    translate_exp(exp->children[2]->children[0], tp);
                    check_refer(exp->children[2]->children[0], tp);
                    append_tac(new Tac(Tac::WRITE, "WRITE", tp));
                } else {
                    list<string> args;
                    translate_args(exp->children[2], &args);
                    for (auto itr = args.begin(); itr != args.end(); ++itr) {
                        append_tac(new Tac(Tac::ARG, "ARG", *itr));
                    }
                    append_tac(new Tac(Tac::CALL, "CALL", child->value, place));
                }
            }
        }
    } else if (child->name == "Exp") {
        ast_node* sibling = exp->children[1];
        string sname = sibling->name;
        if (sname == "ASSIGN") { // Exp ASSIGN Exp
            string t1 = Tmp();
            translate_exp(child, t1);
            check_refer(child, t1);

            string t2 = Tmp();
            translate_exp(exp->children[2], t2);
            check_refer(exp->children[2], t1);

            append_tac(new Tac(Tac::ASSIGN, "ASSIGN", t2, t1));
            append_tac(new Tac(Tac::ASSIGN, "ASSIGN", t1, place));
        } else if (sname == "PLUS" || sname == "MINUS" || sname == "MUL" || sname == "DIV") {
            // Arithmetic
            string t1 = Tmp();
            translate_exp(child, t1);
            check_refer(child, t1);

            string t2 = Tmp();
            translate_exp(exp->children[2], t2);
            check_refer(exp->children[2], t2);

            append_tac(new Tac(Tac::ARITH, exp->children[1]->value, t1, t2, place));
        }
        else if (sname == "LT" || sname == "LE" || sname == "GT" || sname == "GE" ||
                 sname == "NE" || sname == "EQ" || sname == "AND" || sname == "OR") {
            string lb1 = Var();
            string lb2 = Var();
            append_tac(new Tac(Tac::ASSIGN, "ASSIGN", "#0", place));
            translate_cond_exp(exp, lb1, lb2);
            append_tac(new Tac(Tac::LABEL, "LABEL", lb1));
            append_tac(new Tac(Tac::ASSIGN, "ASSIGN", "#1", place));
            append_tac(new Tac(Tac::LABEL, "LABEL", lb2));
        } else if (sname == "DOT") {
            // Exp Dot ID
            string t1 = Tmp();
            translate_exp(exp->children[0], t1);

            Type* ttype = ExpressionEntry(exp->children[0]);

            int offset = dynamic_cast<StructureType *>(ttype)->structure_offset(exp->children[2]->value);

            if (ttype->is_refer) {
                string t2 = Tmp();
                if (offset) append_tac(new Tac(Tac::ARITH, "+", t1, "#" + to_string(offset), t2));
                else append_tac(new Tac(Tac::ASSIGN, "ASSIGN", t1, t2));
                t1 = t2;
            }
            else {
                if (offset) {
                    string t2 = Tmp();
                    append_tac(new Tac(Tac::ARITH, "+", t1, "#" + to_string(offset), t2));
                    t1 = t2;
                }
            }
            place = t1;
        } else if (sname == "LB") {
            // Exp LB Exp RB
            string t1 = Tmp();
            translate_exp(exp->children[0], t1);
            string t2 = Tmp();
            translate_exp(exp->children[2], t2);

            Type* base = ExpressionEntry(exp);
            string t3 = Tmp();
            append_tac(new Tac(Tac::ARITH, "*", t2, "#" + to_string(base->type_size()), t3));

            if (typeid(base) == typeid(ArrayType)) {
                append_tac(new Tac(Tac::ARITH, "+", t1, t3, place));
            } else {
                string t4 = Tmp();
                append_tac(new Tac(Tac::ARITH, "+", t1, t3, t4));
                place = t4;
            }
        } else if (child->name == "MINUS") {
            string tp = Tmp();
            translate_exp(exp->children[1], tp);
            append_tac(new Tac(Tac::ARITH, "-", "#0", tp, place));
        } else if (child->name == "LP") {
            // LP Exp RP
            translate_exp(exp->children[1], place);
        }

    }
}

/**
 * Args: Exp COMMA Args
 * Args: Exp
 */
void translate_args(ast_node* Args, list<string>* arg_list) {
    string tp = Tmp();
    translate_exp(Args->children[0], tp);
    arg_list->push_front(tp);
    if (Args->children.size() > 2)
        translate_args(Args->children[2], arg_list);
}

void translate_cond_exp(ast_node *exp, string lb_t, string lb_f){
    ast_node *child = exp->children[0];
    if (child->name == "EXP") {
        string sname = exp->children[1]->name;
        if (sname == "AND") {
            string lb1 = Label();
            translate_cond_exp(exp->children[0], lb1, lb_f);
            append_tac(new Tac(Tac::LABEL, "LABEL", lb1));
            translate_cond_exp(exp->children[2], lb_t, lb_f);
        } else if (sname == "OR") {
            string lb1 = Label();
            translate_cond_exp(exp->children[0], lb_t, lb1);
            append_tac(new Tac(Tac::LABEL, "LABEL", lb1));
            translate_cond_exp(exp->children[2], lb_t, lb_f);
        } else {
            string t1 = Tmp();
            translate_exp(exp->children[2], t1);

            string t2 = Tmp();
            translate_exp(exp->children[2], t2);

            append_tac(new Tac(Tac::IF, exp->children[1]->value, t1, t2, lb_t));
            append_tac(new Tac(Tac::GOTO, "GOTO", lb_f));
        }
    } else if (child->name == "NOT") {
        translate_cond_exp(exp->children[1], lb_f, lb_t);
    } else if (child->name == "LP") {
        translate_cond_exp(exp->children[1], lb_t, lb_f);
    }
}

/**
 * Dec: VarDec
 * Dec: VarDec ASSIGN Exp
 */
void ir_dec(ast_node *node, Type *type){
    Tac *tac = ir_var_dec(node->children[0], type); // 对vardec的部分 generate
    put_ir(tac->operands[ARG2], tac->append_self()); //把变量放进变量表
    if(node->children_num > 1){ // 第二种情况，右边有表达式
        string tp = Tmp();
        translate_exp(node->children[2], tp);
        check_refer(node->children[2], tp);
        append_tac(new Tac(Tac::ASSIGN, "ASSIGN", tp, tac->operands[RESULT]));
    }
}

/**
 * VarDec: ID
 * VarDec: VarDec LB INT RB
 */
Tac* ir_var_dec(ast_node *node, Type* type){
    vector<ast_node*> ast_vector;
    ast_vector.push_back(node);
    vector<int> int_vector;
    string name;
    while (!ast_vector.empty()) {
        ast_node *now = ast_vector.back();
        if (now->children[0]->name == "ID") { // dec var
            name = now->children[0]->value;
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

    string v = Var();
    if (int_vector.size()) { // array
        return new Tac(Tac::DEC, "DEC", v, int_vector, name);
    } else if (equalType(type, new StructureType()), type) { // structure
        return new Tac(Tac::DEC, "DEC", v, vector<int>{}, name);
    } else {
        Tac* tac = new Tac(Tac::ASSIGN,"ASSIGN", val2str(0).c_str(), v);
        tac->operands[ARG2] = name;
        return tac;
    }
}


/**
 * VarList: ParamDec COMMA VarList
 * VarList: ParamDec
 */
void ir_var_list(ast_node *node){
    vector<ast_node *> vec = {node->children[0]};
    while(node->children_num > 1){
        node = node->children[2];
        vec.push_back(node->children[0]);
    }
    while(!vec.empty()){
        ir_param_dec(vec.back());
        vec.pop_back();
    }
}

/**
 * ParamDec: Specifier VarDec
 */
void ir_param_dec(ast_node *node){
    Type *type = ir_specifier(node->children[0]);
    Tac *tac = ir_var_dec(node->children[1], type);
    tac->tac_type = Tac::PARAM;
    tac->op = "PARAM";
    put_ir(tac->operands[ARG2], append_tac(tac));
}
