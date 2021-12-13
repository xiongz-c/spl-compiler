#include "semantic.hpp"
#include "list"
#define ARG1 0
#define ARG2 1
#define RESULT 2
using namespace std;

unordered_map<string,int> ref_map;
unordered_map<string, string> cond_reverse {
   {"==","!="}, {"!=","=="}, {">", "<="}, {"<=",">"}, {"<",">="}, {">=","<"}
};

int tmp_cnt, var_cnt, label_cnt;

string Tmp(){return "t" + to_string(++tmp_cnt);}
string Var(){return "v" + to_string(++var_cnt);}
string Label(){return "label" + to_string(++label_cnt);}

int extractINT(string tmp){
    if(tmp == ""){
        return 0;
    }
    if(tmp[0] =='#'){
        string res = tmp.substr(1);
        return atoi(res.c_str());
    }
    return 0;
}

Type* get_array_type(Type* type);
int cal_array_size(Type *type);
int cal_struct_size(Type *type);
Type* get_array_dim(Type* type, vector<int> * vec);

int cal_array_size(Type *type){
    if(dynamic_cast<ArrayType*>(type) == NULL) return 0;
    vector<int> * dim = new vector<int>;
    Type* root_type = get_array_dim(type, dim);
    int cnt = 1;
    for(int i = 0; i < dim->size() ;i++){
        cnt *= (*dim)[i];
    }
    if(dynamic_cast<StructureType*>(root_type) != NULL){
        return cnt* cal_struct_size(root_type);
    }else{
        return cnt*4;
    }

}


int cal_struct_size(Type *type){
    if(dynamic_cast<StructureType*>(type) == NULL) return 0;
    int res = 0;
    StructureType* tmp = dynamic_cast<StructureType*>(type);
    for(int i = 0; i < tmp->field_size ;i++){
        Type* child = tmp->fields[i];
        string str_Array = "Array";
        string str_structure = "Structure";
        if (child->name == "Primitive_int"){
            res += 4;
        }else if(child->name.compare(0,str_structure.size(),str_structure) == 0){
            res += cal_struct_size(child);
        }else if (child->name.compare(0,str_Array.size(),str_Array) == 0){
            res += cal_array_size(child);
        }
    }
    return res;
}

void remove_tmp(){tmp_cnt--;}
float str_to_num(string value, bool type){
    if (!type) // DEFAULT: parse to int when false
        return atoi(value.c_str());
    else // else parse to float
        return atof(value.c_str());
}


class Tac {
public:
    string op;
    string operands[3]; // {ARG1,ARG2,RESULT}
    vector<int> suffix; // suffix mem
    vector<int> sizes;
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
    }
    Tac(TacType tac_type, const string& op, const string& arg1) {
        this->tac_type = tac_type;
        this->op = op;
        this->operands[RESULT] = arg1;
    }
    Tac(TacType tac_type, const string& op, const string& arg1, const string& res) {
        this->tac_type = tac_type;
        this->op = op;
        this->operands[ARG1] = arg1;
        this->operands[RESULT] = res;
    }
    Tac(TacType tac_type, const string& op, const string& arg1, const string& arg2, const string& res) {
        this->tac_type = tac_type;
        this->op = op;
        this->operands[ARG1] = arg1;
        this->operands[ARG2] = arg2;
        this->operands[RESULT] = res;
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
        this->operands[ARG1] = std::to_string(cal_array_size(glo_symbolTable.searchEntry(name,"VAR")->type));
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
                printf("%s %s :\n",op.c_str(), operands[RESULT].c_str());
                break;
            case LABEL: // op: LABEL
                printf("%s %s :\n", op.c_str(), operands[RESULT].c_str());
                break;
            case EXIT: // last line of function. for ir optimize
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
extern SymbolTable glo_symbolTable; // 扩展符号表 for proj3
string Tac::append_self() {
    if(this->tac_type == Tac::ARITH || this->tac_type == Tac::ASSIGN){
        if(this->operands[RESULT].empty() || this->operands[ARG1].empty()){
            return "";
        }
    }
    tac_vector.push_back(this);
    if (this->tac_type == Tac::FUNC) return std::to_string(tac_vector.size() - 1);
    return this->operands[RESULT];
}

string append_tac(Tac *tac) {
    if(tac->tac_type == Tac::ARITH || tac->tac_type == Tac::ASSIGN){
        if(tac->operands[RESULT].empty() || tac->operands[ARG1].empty()){
            return "";
        }
    }
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
void put_ir(string name, string vid){value_info[name] = vid;}

// 根据名字取变量
string get_ir(string name){return value_info[name];}


void display_value_info(){
    for (auto itr = value_info.begin(); itr != value_info.end(); ++itr) {
        cout << itr->first << "  " << itr->second << endl;
    }
}

void check_refer(ast_node* exp, string& place) {
    if (exp->children[0]->name == "Exp" && exp->children.size() > 1) {
        ast_node* sibling = exp->children[1];
        if (sibling->name == "LB" || sibling->name == "DOT") {
            place = "*" + place;
        }
    }
}

void ir_init() {
    for(auto item = glo_symbolTable.symbolTableInstance.begin(); item != glo_symbolTable.symbolTableInstance.end(); item++){
        ref_map[item->first] = 0;
    }
    tac_vector.clear();
    value_info.clear();
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
Type* ir_structSpecifier(ast_node * node);
void ir_def(ast_node *node);
void ir_dec_list(ast_node *node, Type *type);
void ir_stmt(ast_node *node);
void ir_stmt_list(ast_node *node);
void ir_dec(ast_node *node, Type *type);
Tac* ir_var_dec(ast_node *node, Type* type, bool isParam);
void ir_var_list(ast_node *node);
void ir_param_dec(ast_node *node);
void translate_cond_exp(ast_node *exp, string lb_t, string lb_f);
void translate_exp(ast_node *exp, string& place);
void translate_args(ast_node *exp, list<string>* arg_list);
int count_structure( StructureType* st_type);
int handle_array_location(ast_node* node);
string get_real_location(ast_node* node, vector<int> * vec);
int cal_real_offset( vector<int> * vec,vector<int> * dim );
void optimize();
void optimize_const();
void reduce_tmp();
pair<string,int> check_optimizable();
unordered_map<string,int> tmp_occur;
bool check_exit_tac(vector<Tac*>::iterator it){
    if (it == tac_vector.end() || (*it)->tac_type == Tac::EXIT)
        return true;
    return false;
}

int cal_op_res(int a1,int a2, string op){
    if(op == "-"){
        return a1-a2;
    }else if(op == "+"){
        return a1+a2;
    }else if(op == "*"){
        return a1*a2;
    }else if(op == "/"){
        return a1/a2;
    }
    return 0;
}

pair<string,int> check_optimizable(){
    Tac* tac;
    auto it = tac_vector.begin();
    int cnt = 1;
    while (it != tac_vector.end()) {
        tac = *it;
        if(tac->operands[RESULT][0] == 't' && (tac->tac_type == Tac::ASSIGN || tac->tac_type == Tac::ARITH) ){
            // t0 = v1 - v1
            if(tac->op == "-" && tac->operands[ARG1] == tac->operands[ARG2]){

                tac_vector.erase(it);
                return make_pair(tac->operands[RESULT],0);
            }

            //cout << "cnt no " << cnt << tac->operands[ARG1] << " xxx " << tac->operands[ARG2] << endl;
            // both const
            if(tac->operands[ARG1] !="" && tac->operands[ARG1][0] == '#' && tac->operands[ARG2] !="" && tac->operands[ARG2][0] == '#'){
                int tmp1 = extractINT(tac->operands[ARG1]);
                int tmp2 = extractINT(tac->operands[ARG2]);
                tac_vector.erase(it);
                return make_pair(tac->operands[RESULT],cal_op_res(tmp1,tmp2,tac->op));
            }

            if(tac->operands[ARG2] == "" && tac->operands[ARG1] !="" && tac->operands[ARG1][0] == '#'){
                tac_vector.erase(it);
                return make_pair(tac->operands[RESULT], extractINT(tac->operands[ARG1]));
            }

//
//            int con1 = 0;
//            int con2 = 0;
//            if(tac->operands[ARG1] == "" ||tac->operands[ARG1][0] =='#'){
//                con1 = 1;
//            }
//            if(tac->operands[ARG2] == "" ||tac->operands[ARG2][0] =='#'){
//                con2 = 1;
//            }
//            if(con1 && con2){
//                cout << "No: " << cnt  << " can be optimize : " << tac->operands[RESULT] << " ARG1 " << tac->operands[ARG1]  << " ARG2 " << tac->operands[ARG2]  << endl;
//            }

        }
        it++;
        cnt++;
    }
    return make_pair("",0);
}

void clean_tac_list( pair<string,int> op_info){
    Tac* tac;
    auto it = tac_vector.begin();
    while (it != tac_vector.end()) {
        tac = *it;
        if(tac->operands[ARG1] == op_info.first){
            tac->operands[ARG1] = "#"+to_string(op_info.second);
        }
        if(tac->operands[ARG2] == op_info.first){
            tac->operands[ARG2] = "#"+to_string(op_info.second);
        }
        if(tac->operands[RESULT] == op_info.first){
            tac->operands[RESULT] = "#"+to_string(op_info.second);
        }
        it++;
    }
}

void optimize_const(){
    pair<string,int> op_info = check_optimizable();
    while(op_info.first != ""){
        clean_tac_list(op_info);
        op_info = check_optimizable();
    }
    for(auto it = tac_vector.begin(); it != tac_vector.end(); it++){
        Tac *tac = *it;
        if(tac->operands[ARG1] != "" && tac->operands[ARG1][0] == 't'){
            tmp_occur[tac->operands[ARG1]]++;
        }
        if(tac->operands[ARG2] != "" && tac->operands[ARG2][0] == 't'){
            tmp_occur[tac->operands[ARG2]]++;
        }
        if(tac->operands[RESULT] != "" && tac->operands[RESULT][0] == 't'){
            tmp_occur[tac->operands[RESULT]]++;
        }
    }
    reduce_tmp();
    return;
}

void reduce_tmp(){
    auto it1 = tac_vector.begin();
    auto it2 = it1+1;
    while (it1 != tac_vector.end() && it2 != tac_vector.end()){
        Tac * tac1 = *it1;
        Tac * tac2 = *it2;
        if(tac1->tac_type == Tac::ARITH && tac2->tac_type == Tac::ASSIGN){
            if(tac1->operands[RESULT] == tac2->operands[ARG1] && tac2->operands[ARG1] != ""
            && tac2->operands[ARG1][0] == 't'
            && tmp_occur[tac2->operands[ARG1]] == 2){
                tac2->operands[ARG1] = tac1->operands[ARG1];
                tac2->operands[ARG2] = tac1->operands[ARG2];
                tac2->tac_type = Tac::ARITH;
                tac2->op = tac1->op;
                tac_vector.erase(it1);
                it1 = it2;
                it2++;
                continue;
            }
        }
        it1++;
        it2++;
    }
}


/**
 * Program: ExtDefList
*/
void ir_starter(ast_node *root) {
    ir_init();
    ir_ext_def_list(root->children[0]);
    optimize();
    optimize_const();
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
        //TODO check whether need a func id
        ir_comp_stmt(node->children[2]);
        append_tac(new Tac(Tac::EXIT));
    }
}

void ir_ext_dec_list(ast_node *node, Type *type){
    Tac *tac = ir_var_dec(node->children[0], type, false);
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
        //TODO tobe checked whether a global map?
        type = ir_structSpecifier(node->children[0]);
    }
    return type;
}


Type* ir_structSpecifier(ast_node * node){
    SymbolElement* res = glo_symbolTable.searchEntry(node->children[1]->value,"STRUCT");
    return res->type;
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

            SymbolElement * target_se = glo_symbolTable.searchEntry(child->value,"VAR");
            Type* expType = target_se->type;
            // TODO handle array
            if (ref_map["VAR_"+child->value]) {
                place = "&" + place;
            }
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
            string left_name =  exp->children[0]->children[0]->value;
            Type* ttype = glo_symbolTable.searchEntry(left_name,"VAR")->type;
            int offset = dynamic_cast<StructureType *>(ttype)->structure_offset(exp->children[2]->value);
            string t2 = Tmp();
            if (offset) append_tac(new Tac(Tac::ARITH, "+", t1, "#" + to_string(offset), t2));
            else append_tac(new Tac(Tac::ASSIGN, "ASSIGN", t1, t2));
            t1 = t2;
            place = t1;
        } else if (sname == "LB") {
            // Exp LB Exp RB
            vector<int> * vec = new vector<int>;
            vector<int> * dim = new vector<int>;
            string var_name = get_real_location(exp,vec);

            Type* tmp = glo_symbolTable.searchEntry(var_name,"VAR")->type;
            Type* root_type = get_array_dim(tmp,dim);
            string t1 = Tmp();
            t1 = get_ir(var_name);
            if (ref_map["VAR_"+var_name]) {
                t1 = "&" + t1;
            }
            string t2 = Tmp();
            string str_Array = "Array";
            string str_structure = "Structure";

            int times = 0;
            if (root_type->name == "Primitive_int"){
                times += 4;
            }else if(root_type->name.compare(0,str_structure.size(),str_structure) == 0){
                times += cal_struct_size(root_type);
            }
            append_tac(new Tac(Tac::ARITH,"+",t1,"#"+ to_string(times*cal_real_offset(vec,dim)),t2));
            place = t2;
        }
    }else if (child->name == "MINUS") {
        string tp = Tmp();
        translate_exp(exp->children[1], tp);
        append_tac(new Tac(Tac::ARITH, "-", "#0", tp, place));
    } else if (child->name == "LP") {
        // LP Exp RP
        translate_exp(exp->children[1], place);
    }
}

string get_real_location(ast_node* node, vector<int> * vec){
    if(node->children[0]->name == "ID")return node->children[0]->value;
    if(node->children[2]->children[0]->name == "INT"){
        vec->push_back(atoi(node->children[2]->children[0]->value.c_str()));
    }


    return get_real_location(node->children[0],vec);
}

Type* get_array_dim(Type* type, vector<int> * vec){
    if( dynamic_cast<ArrayType*>(type) == NULL){
        return type;
    }else{
        vec->push_back(dynamic_cast<ArrayType*>(type)->size);
        return get_array_dim(dynamic_cast<ArrayType*>(type)->base,vec);
    }
}

Type* get_array_type(Type* type){
    if( dynamic_cast<ArrayType*>(type) == NULL){
        return type;
    }else{
        return get_array_type(dynamic_cast<ArrayType*>(type)->base);
    }
}


int cal_real_offset( vector<int> * vec,vector<int> * dim ){
    int res = 0;
    int po = 1;
    for (int i = vec->size()-1; i >0  ; i--) {
        int tmp = (*vec)[i];
        for(int j = po; j < dim->size() ; j++){
            tmp *= (*dim)[j];
        }
        po++;
        res += tmp;
    }
    res += (*vec)[0];
    return res;
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
    if (child->name == "Exp") {
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
            translate_exp(exp->children[0], t1);

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
    Tac *tac = ir_var_dec(node->children[0], type, false); // 对vardec的部分 generate
    put_ir(tac->operands[ARG2], tac->operands[RESULT]); //把变量放进变量表
    if (tac->tac_type == Tac::DEC) tac->append_self();
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
Tac* ir_var_dec(ast_node *node, Type* type,bool isParam){
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
        if(!isParam)ref_map["VAR_"+name] = 1;
        return new Tac(Tac::DEC, "DEC", v, int_vector, name);
    } else if (type->name == "Structure") { // structure
        if(!isParam)ref_map["VAR_"+name] = 1;
        StructureType* st_type = dynamic_cast<StructureType*>(type);
        int cnt = count_structure(st_type);
        return new Tac(Tac::DEC, "DEC", to_string(cnt),name,v);
    } else {
        Tac* tac = new Tac(Tac::ASSIGN,"ASSIGN", "#0", v);
        tac->operands[ARG2] = name;
        return tac;
    }
}

int count_structure( StructureType* st_type){
    int cnt = 0;
    for(int i = 0; i < st_type->field_size ; i++){
        cnt += 4;
    }
    return cnt;
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
    if(!vec.empty()){
        for (int i = 0; i < vec.size(); ++i) {
            ir_param_dec(vec[i]);
        }
    }
    vec.clear();
//    while(!vec.empty()){
//        ir_param_dec(vec.back());
//        vec.pop_back();
//    }
}

/**
 * ParamDec: Specifier VarDec
 */
void ir_param_dec(ast_node *node){
    Type *type = ir_specifier(node->children[0]);
    Tac *tac = ir_var_dec(node->children[1], type, true);
    tac->tac_type = Tac::PARAM;
    tac->op = "PARAM";
    put_ir(tac->operands[ARG2], append_tac(tac));
}

void optimize(){
    // optimize goto
    auto itr = tac_vector.begin();
    while(itr != tac_vector.end()) {
        Tac* tac = *itr;
        auto itr_tmp = itr;
        if (tac->tac_type == Tac::GOTO) { // goto optimize
            auto itr_tmp2 = ++itr_tmp;
            if (!check_exit_tac(itr_tmp2)) {
                if ((*itr_tmp2)->tac_type == Tac::LABEL && tac->operands[RESULT] == (*itr_tmp2)->operands[RESULT]){
                    tac_vector.erase(itr++);
                    continue;
                }
            }
        } else if (tac->tac_type == Tac::IF) { // if cond goto optimize
            auto itr_tmp3 = ++itr_tmp;
            if (!check_exit_tac(itr_tmp3)) {
                auto itr_tmp4 = ++itr_tmp;
                if (!check_exit_tac(itr_tmp4)) {
                    if ((*itr_tmp3)->tac_type == Tac::GOTO
                        && (*itr_tmp4)->tac_type == Tac::LABEL
                        && tac->operands[RESULT] == (*itr_tmp4)->operands[RESULT]) {
                        tac->operands[RESULT] = (*itr_tmp3)->operands[RESULT];
                        string rever_op = cond_reverse[tac->op];
                        tac->op = rever_op;
                        tac_vector.erase(itr_tmp3);
                    }
                }
            }
        }
        itr++;
    }

    // optimize label
    Tac* tac;
    list<string> labels;
    for (auto itr = tac_vector.begin(); itr != tac_vector.end(); ++itr) {
        tac = *itr;
        if (tac->tac_type == Tac::GOTO)
            labels.push_back(tac->operands[RESULT]);
        else if (tac->tac_type == Tac::IF)
            labels.push_back(tac->operands[RESULT]);
    }
    auto it = tac_vector.begin();
    while (it != tac_vector.end()) {
        tac = *it;
        if (tac->tac_type == Tac::LABEL) {
            if (find(labels.begin(), labels.end(), tac->operands[RESULT]) == labels.end()) {
                tac_vector.erase(it++);
                continue;
            }
        }
        it++;
    }

    // optimize read
    it = tac_vector.begin();
    while (it != tac_vector.end()) {
        tac = *it;
        if ( tac->tac_type == Tac::READ ) {
            Tac *tac2 = *(++it);
            if (tac2->tac_type == Tac::ASSIGN && tac2->operands[ARG1] == tac->operands[RESULT]) {
                tac->operands[RESULT] = tac2->operands[RESULT];
                tac_vector.erase(it);
                continue;
            }
        }
        it++;
    }


}
