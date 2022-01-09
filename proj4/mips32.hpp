#ifndef MIPS_H
#define MIPS_H
#include "tac.hpp"
using namespace std;
void emit_preamble();
void emit_read_function();
void emit_write_function();
void _mips_printf(const char *fmt, ...);
void _mips_iprintf(const char *fmt, ...);
extern vector<Tac*> tac_vector;

enum MipsOp {
    MIPS_LABEL,
    MIPS_LI,
    MIPS_LA,
    MIPS_MOVE,
    MIPS_ADDI,
    MIPS_ADD,
    MIPS_SUB,
    MIPS_MUL,
    MIPS_DIV,
    MIPS_MFLO,
    MIPS_LW,
    MIPS_SW,
    MIPS_J,
    MIPS_JAL,
    MIPS_JR,
    MIPS_BLT,
    MIPS_BLE,
    MIPS_BGT,
    MIPS_BGE,
    MIPS_BNE,
    MIPS_BEQ,
    MIPS_SYSCALL
};

typedef enum {
    zero, at, v0, v1, a0, a1, a2, a3,
    t0, t1, t2, t3, t4, t5, t6, t7,
    s0, s1, s2, s3, s4, s5, s6, s7,
    t8, t9, k0, k1, gp, sp, fp, ra, NUM_REGS, NO_REG
} Register;

unordered_map<Register, string> regToStr = {
        {zero,"$zero"},
        {at,"$at"},
        {v0,"$v0"},{v1,"$v1"},
        {a0,"$a0"}, {a1,"$a1"}, {a2,"$a2"}, {a3,"$a3"},
        {t0,"$t0"}, {t1,"$t1"}, {t2,"$t2"}, {t3,"$t3"}, {t4,"$t4"}, {t5,"$t5"}, {t6,"$t6"}, {t7,"$t7"},
        {s0,"$s0"}, {s1,"$s1"}, {s2,"$s2"}, {s3,"$s3"}, {s4,"$s4"}, {s5,"$s5"}, {s6,"$s6"}, {s7,"$s7"},
        {t8,"$t8"}, {t9,"$t9"}, {k0,"$k0"}, {k1,"$k1"}, {gp,"$gp"}, {sp,"$sp"}, {fp,"$fp"}, {ra,"$ra"},
        {NO_REG,"NO_REG"}
};

Register localAllocate();
int offset_po_fp = 0;
struct AddressDesc{
    string name;
    Register reg_num = NO_REG; // the number of register eg:  t0 is 8
    int offset = -1; // offset to $fp
    bool isPlus;

    AddressDesc(string name, Register reg, int offset, bool isPlus) {
        this->name = name;
        this->reg_num = reg;
        this->offset = offset;
        this->isPlus = isPlus;
    }

};

AddressDesc * CONST_ADR = new AddressDesc("CONST",NO_REG,0,false);

unordered_map<string, AddressDesc*> nameToAddress;

struct RegDesc {    // the register descriptor
    string name;
    string var;
    bool dirty = false; // value updated but not stored
    bool isAllocated = false;
    AddressDesc * adr = NULL;
    /* add other fields as you need */
} regs[NUM_REGS];

class Inst {
public:
    string res;
    string arg1;
    string arg2;
    MipsOp op;

    string debug_info; // comment for debug


    Inst(MipsOp op){
        this->op = op;
    }

    Inst(MipsOp op, const string &res){
        this->op = op;
        this->res = res;
    }

    Inst(MipsOp op, const string &res, const string &arg1){
        this->op = op;
        this->res = res;
        this->arg1 = arg1;
    }

    Inst(MipsOp op, const string &res, const string &arg1, const string &arg2){
        this->op = op;
        this->res = res;
        this->arg1 = arg1;
        this->arg2 = arg2;
    }

    void print_inst() {
        if (this->op != MIPS_LABEL) {
            cout << "  ";
        }
        cout << this->to_string();
        if (!debug_info.empty()) {
            cout << "\t# " << debug_info;
        }
        cout << endl;
    }

    void set_debug(const string &debug_info) {
        this->debug_info = debug_info;
    }

    string to_string() {
        switch (this->op) {
            case MIPS_LABEL:
                return this->res + ":";
            case MIPS_LI:
                return "li " + res + ", " + arg1;
            case MIPS_LA:
                return "la " + res + ", " + arg1;
            case MIPS_MOVE:
                return "move " + res + ", " + arg1;
            case MIPS_ADDI:
                return "addi " + res + ", " + arg1 + ", " + arg2;
            case MIPS_ADD:
                return "add " + res + ", " + arg1 + ", " + arg2;
            case MIPS_SUB:
                return "sub " + res + ", " + arg1 + ", " + arg2;
            case MIPS_MUL:
                return "mul " + res + ", " + arg1 + ", " + arg2;
            case MIPS_DIV:
                return "div " + arg1 + ", " + arg2;
            case MIPS_MFLO:
                return "mflo " + res;
            case MIPS_LW:
                return "lw " + res + ", " + arg2 + '(' + arg1 + ')';
            case MIPS_SW:
                return "sw " + res + ", " + arg2 + '(' + arg1 + ')';
            case MIPS_J:
                return "j " + res;
            case MIPS_JAL:
                return "jal " + res;
            case MIPS_JR:
                return "jr " + res;
            case MIPS_BLT:
                return "blt " + arg1 + ", " + arg2 + ", " + res;
            case MIPS_BLE:
                return "ble " + arg1 + ", " + arg2 + ", " + res;
            case MIPS_BGT:
                return "bgt " + arg1 + ", " + arg2 + ", " + res;
            case MIPS_BGE:
                return "bge " + arg1 + ", " + arg2 + ", " + res;
            case MIPS_BNE:
                return "bne " + arg1 + ", " + arg2 + ", " + res;
            case MIPS_BEQ:
                return "beq " + arg1 + ", " + arg2 + ", " + res;
            case MIPS_SYSCALL:
                return "syscall";
            default:
                return "";
        }
    }
};

class Mips {
public:
    list<Inst*> instructions;

    void output() {
        for (auto it: instructions) {
            it->print_inst();
        }
    }

    void put_Inst(Inst * ins){
        instructions.push_back(ins);
    }

    void extendStack(int bytes){
        this->put_Inst(new Inst(MIPS_ADDI, "$sp", "$sp", to_string(-bytes)));
    }

    void recoverStack(int bytes){
        this->put_Inst(new Inst(MIPS_ADDI, "$sp", "$sp", to_string(bytes)));
    }


};
Register getRegister(string name,Mips *mips);


class Block {
public:
    vector<Tac*> ir;
    vector<Tac*> param_tac_list; //如果block是一个函数，需要计算参数的个数，确定fp的offset
    vector<Tac*> arg_tac_list; 
    Mips* mips;
    bool isStored = false;

    Block(Tac* ld) {this->ir.push_back(ld);}

    // 打印block里的ir指令, 用于debug
    void print() {
        cout <<"========block begin========" << endl;
        for (auto item: ir) item->to_string();
        cout <<"========block end========" << endl;
    }

    // 设置当前所属的mips类
    void set_mips(Mips* m) {
        this->mips = m;
    }

    // 生成callee
    void generateFunction(Mips* mips, Tac* tac);
    void generateAssign(Mips* mips, Tac* tac);
    void generateIfBranch(Mips* mips, Tac* tac);
    void generateCall(Mips* mips, Tac* tac);
    void generateReturn(Mips* mips, Tac* tac);
    void generateArith(Mips* mips, Tac* tac);
    void saveRegister(Mips * mips);
    void emit_read_function(Mips * mips, Tac* tac);
    void emit_write_function(Mips * mips, Tac* tac);
    void reloadRegister(Mips * mips);

    // 向当前所属的mips类添加指令
    void gen_code() {
        //this->print();
        this->analysis();
        auto itr = ir.begin();
        while(itr != ir.end()){
            Tac* tac = *(itr);
            if(tac->tac_type == Tac::FUNC ){
                generateFunction(this->mips,tac);
            }else if(tac->tac_type == Tac::ASSIGN){
                generateAssign(this->mips,tac);
            }else if (tac->tac_type == Tac::IF){
                generateIfBranch(this->mips,tac);
            }else if (tac->tac_type == Tac::CALL){
                generateCall(this->mips,tac);
            }else if (tac->tac_type == Tac::GOTO){
                this->saveRegister(mips);
                this->isStored = true;
                this->mips->put_Inst(new Inst(MIPS_J, tac->operands[RESULT]));
            }else if (tac->tac_type == Tac::RETURN){
                generateReturn(this->mips,tac);
            }else if (tac->tac_type == Tac::ARITH){
                generateArith(this->mips,tac);
            }else if (tac->tac_type == Tac::LABEL){
                this->mips->put_Inst(new Inst(MIPS_LABEL, tac->operands[RESULT]));
            }else if (tac->tac_type == Tac::READ){
                emit_read_function(this->mips,tac);
            }else if (tac->tac_type == Tac::WRITE){
                emit_write_function(this->mips,tac);
            }
            itr++;
        }
        if(!this->isStored){
            this->saveRegister(this->mips);
        }
    }

    void analysis(){
        // 清理nameToAddress 的table
        auto start_po = ir.begin();
        if(start_po != ir.end()){
            Tac * tac_po = *(start_po);
            if(tac_po->tac_type == Tac::FUNC){
                nameToAddress.clear();
                offset_po_fp = 0;
            }
        }
        Register reg = NUM_REGS;
        for(int i = 0 ; i < (int)reg ; i++){
            regs[i].isAllocated = false;
            regs[i].adr = NULL;
            regs[i].dirty = false;
        }



        for (const auto &item: nameToAddress) {
            AddressDesc *adr = item.second;
            regs[adr->reg_num].isAllocated = false;
            regs[adr->reg_num].dirty = false;
            adr->reg_num = NO_REG;
        }

        
        auto itor = ir.end();
        while(itor != ir.begin()){
            itor--;
            Tac * tac = *(itor);
            if(tac->tac_type == Tac::PARAM){
                param_tac_list.push_back(tac);
            }else if(tac->tac_type == Tac::ARG){
                arg_tac_list.push_back(tac);
            }
        }

    }
};

list<Block*> block_list;

Register localAllocate(){
    for(Register tmp = t0 ; tmp <= t9 ; tmp = (Register)(tmp+1)){
        if(!regs[tmp].isAllocated){
            regs[tmp].isAllocated = true;
            return tmp;
        }
    }
    return NO_REG;
}

Register argAllocate(){
    for(Register tmp = a0 ; tmp <= a3 ; tmp = (Register)(tmp+1)){
        if(!regs[tmp].isAllocated){
            regs[tmp].isAllocated = true;
            return tmp;
        }
    }
    return NO_REG;
}

Register getRegister(string name, Mips *mips){
    auto it = nameToAddress.find(name);
    if ( it != nameToAddress.end()){
        AddressDesc* adr = it->second;
        if (adr != NULL){
            if(adr->reg_num == NO_REG ){
                adr->reg_num = localAllocate();
                regs[adr->reg_num].adr = adr;
                if(adr->offset > 0){
                    if (adr->isPlus ) {
                        mips->put_Inst(new Inst(MIPS_LW, regToStr[adr->reg_num], "$fp", to_string(adr->offset)));
                    } else {
                        mips->put_Inst(new Inst(MIPS_LW, regToStr[adr->reg_num], "$fp", to_string(-adr->offset)));
                    }
                }
            }
        }

        return adr->reg_num;
    }
    Register reg = localAllocate();
    regs[reg].isAllocated = true;
    AddressDesc* address = new AddressDesc(name,reg, -1, false);
    nameToAddress.insert(make_pair(name,address));
    return reg;
}

void Block::generateFunction(Mips* mips, Tac* tac){
    Inst * ins = new Inst(MIPS_LABEL,tac->operands[RESULT].c_str());
    mips->put_Inst(ins);
    mips->put_Inst(new Inst(MIPS_MOVE, "$fp", "$sp"));
    int param_size = this->param_tac_list.size();
    int offset_cnt = param_size + 2;
    reverse(this->param_tac_list.begin(),this->param_tac_list.end());
    for (auto item = this->param_tac_list.begin(); item != this->param_tac_list.end() ; ++item) {
        Register tmp = argAllocate();
        regs[tmp].isAllocated = true;
        offset_cnt--;
        Tac * tac = (*item);
        string name = tac->operands[RESULT];
        if(regs[tmp].adr == NULL){
            regs[tmp].isAllocated = true;
            auto it = nameToAddress.find(name);
            if( it != nameToAddress.end()){

            }else{
                AddressDesc* address = new AddressDesc(name,tmp,offset_cnt*4, true);
                nameToAddress.insert(make_pair(name,address));
                regs[tmp].adr = address;
                this->mips->put_Inst(new Inst(MIPS_LW,regToStr[tmp],"$fp", to_string(address->offset)));
            }
        }

    }


}

//TODO 非立即数赋值没写
void Block::generateAssign(Mips* mips, Tac* tac){
    Register tmp = getRegister(tac->operands[RESULT],mips);
    regs[tmp].dirty = true;
    // 若 立即数 赋值
    if(tac->operands[ARG1][0]=='#'){
        mips->put_Inst(new Inst(MIPS_LI,regToStr[tmp],tac->operands[ARG1].substr(1)));
    }else {
        Register dest = getRegister(tac->operands[ARG1],mips);
        mips->put_Inst(new Inst(MIPS_MOVE,regToStr[tmp],regToStr[dest]));
    }
}

// 立即数没有特别标注 address
void Block::generateIfBranch(Mips* mips, Tac* tac){
    if(!this->isStored)this->saveRegister(mips);
    this->isStored = true;
    Register r1;
    Register r2;
    if(tac->operands[ARG1][0] == '#'){
        r1 = localAllocate();
        //regs[r1].adr = CONST_ADR;
        mips->put_Inst(new Inst(MIPS_LI,regToStr[r1],tac->operands[ARG1].substr(1)));
    }else{
        r1 = getRegister(tac->operands[ARG1],mips);
    }

    if(tac->operands[ARG2][0] == '#'){
        r2 = localAllocate();
        mips->put_Inst(new Inst(MIPS_LI,regToStr[r2],tac->operands[ARG2].substr(1)));
        //regs[r2].adr = CONST_ADR;
    }else{
        r2 = getRegister(tac->operands[ARG2],mips);
    }
    MipsOp branchOp;
    if(tac->op == ">="){
        branchOp = MIPS_BGE;
    }else if (tac->op == ">"){
        branchOp = MIPS_BGT;
    }else if (tac->op == "<="){
        branchOp = MIPS_BLE;
    }else if (tac->op == "<"){
        branchOp = MIPS_BLT;
    }else if (tac->op == "=="){
        branchOp = MIPS_BEQ;
    }else if (tac->op == "!="){
        branchOp = MIPS_BNE;
    }
    mips->put_Inst(new Inst(branchOp,tac->operands[RESULT],regToStr[r1],regToStr[r2]));
}

void Block::saveRegister(Mips * mips){
    for(auto it = nameToAddress.begin(); it != nameToAddress.end(); it++){
        AddressDesc * adr = it->second;
        if(adr->reg_num != NO_REG && regs[adr->reg_num].dirty){
            regs[adr->reg_num].dirty = false;
            if(adr->offset < 0 ){
                mips->extendStack(4);
                offset_po_fp++;
                adr->offset = offset_po_fp*4;
            }
            if (adr->isPlus) {
                mips->put_Inst(new Inst(MIPS_SW,regToStr[adr->reg_num],"$fp", to_string(adr->offset)));
            } else {
                mips->put_Inst(new Inst(MIPS_SW,regToStr[adr->reg_num],"$fp", to_string(-adr->offset)));
            }
            regs[adr->reg_num].dirty = false;
        }
    }
     
}

void Block::reloadRegister(Mips * mips){
    for (auto &item: nameToAddress) {
        AddressDesc *adr = item.second;
        if (adr->reg_num == NO_REG)
            continue;
        if(adr->offset > 0){
            if (adr->isPlus) {
                mips->put_Inst(new Inst(MIPS_LW,regToStr[adr->reg_num],"$fp", to_string(adr->offset)));
            } else {
                mips->put_Inst(new Inst(MIPS_LW,regToStr[adr->reg_num],"$fp", to_string(-adr->offset)));
            }
        }
    }
}

void Block::generateCall(Mips* mips, Tac* tac){
    if(!this->isStored)this->saveRegister(mips);
    this->isStored = true;
    mips->extendStack(4*this->arg_tac_list.size()+8);
    //this->put_Inst(new Inst(MIPS_ADDI, "$sp", "$sp", to_string(-bytes)));
    mips->put_Inst(new Inst(MIPS_SW, "$fp", "$sp", "0"));
    mips->put_Inst(new Inst(MIPS_SW, "$ra", "$sp", "4"));
    int cnt = 0;
    reverse(arg_tac_list.begin(),arg_tac_list.end());
    for (Tac *tac: arg_tac_list) {
        Register arg_reg;
        if(tac->operands[RESULT][0] == '#'){
            arg_reg = localAllocate();
            mips->put_Inst(new Inst(MIPS_LI, regToStr[arg_reg],tac->operands[RESULT].substr(1)));
        }else{
            arg_reg = getRegister(tac->operands[RESULT],mips);
        }
        mips->put_Inst(new Inst(MIPS_SW, regToStr[arg_reg], "$sp", to_string(cnt * 4 + 8)));
        cnt++;
    }
    this->arg_tac_list.clear();
    
    mips->put_Inst(new Inst(MIPS_JAL, tac->operands[ARG1]));
    mips->put_Inst(new Inst(MIPS_LW, "$ra", "$fp", "4"));
    mips->put_Inst(new Inst(MIPS_LW, "$fp", "$fp", "0"));
    this->reloadRegister(mips);
    Register reg = getRegister(tac->operands[RESULT],mips);
    regs[reg].dirty = true;
    mips->put_Inst(new Inst(MIPS_MOVE, regToStr[reg], "$v0"));
    mips->recoverStack(4 * this->arg_tac_list.size() + 8);

    /** restore register value from memory*/
    // restoreRegisterStatus(mips);
    // Reg *ret = getRegOfSymbol(pInst->target);
    // ret->setDirty();
    // mips->put_Inst(new MIPS_Instruction(MIPS_MOVE, ret->getName(), "$v0"));

    // mips->pop(4 * numOfArgs + 8);
}

void Block::generateReturn(Mips* mips, Tac* tac){
    if (tac->operands[RESULT][0] == '#'){
        mips->put_Inst(new Inst(MIPS_LI, "$v0",tac->operands[RESULT].substr(1)));
    }else {
        Register reg_tmp = getRegister(tac->operands[RESULT],mips);
        mips->put_Inst(new Inst(MIPS_MOVE, "$v0", regToStr[reg_tmp]));
    }
    mips->put_Inst(new Inst(MIPS_MOVE, "$sp", "$fp"));
    mips->put_Inst(new Inst(MIPS_JR, "$ra"));
}

void Block::generateArith(Mips* mips, Tac* tac){
    Register reg1,reg2;
    if(tac->operands[ARG1][0] == '#'){
        reg1 = localAllocate();
        mips->put_Inst(new Inst(MIPS_LI, regToStr[reg1],tac->operands[ARG1].substr(1)));
        regs[reg1].adr = CONST_ADR;
    }else{
        reg1 = getRegister(tac->operands[ARG1],mips);
    }

    if(tac->operands[ARG2][0] == '#'){
        reg2 = localAllocate();
        mips->put_Inst(new Inst(MIPS_LI, regToStr[reg2],tac->operands[ARG2].substr(1)));
        regs[reg2].adr = CONST_ADR;
    }else{
        reg2 = getRegister(tac->operands[ARG2],mips);
    }
    Register dest = getRegister(tac->operands[RESULT],mips);
    if (tac->op == "+"){
        mips->put_Inst(new Inst(MIPS_ADD, regToStr[dest],regToStr[reg1],regToStr[reg2]));
    }else if (tac->op == "-"){
        mips->put_Inst(new Inst(MIPS_SUB, regToStr[dest],regToStr[reg1],regToStr[reg2]));
    }else if (tac->op == "*"){
         mips->put_Inst(new Inst(MIPS_MUL, regToStr[dest],regToStr[reg1],regToStr[reg2]));
    }
}

void Block::emit_read_function(Mips * mips, Tac* tac){
    for(auto &it : nameToAddress){
        AddressDesc * adr = it.second;
        if(adr->reg_num == a0){
           if (adr->isPlus) {
                mips->put_Inst(new Inst(MIPS_SW,"$a0","$fp", to_string(adr->offset)));
            } else {
                mips->put_Inst(new Inst(MIPS_SW,"$a0","$fp", to_string(-adr->offset)));
            }
            regs[a0].dirty = false;
            regs[a0].isAllocated = false;
            adr->reg_num = NO_REG;
        }
    }
    mips->put_Inst(new Inst(MIPS_LI,"$v0","4"));
    mips->put_Inst(new Inst(MIPS_LA, "$a0", "_prmpt"));
    mips->put_Inst(new Inst(MIPS_SYSCALL));
    Register num = getRegister(tac->operands[RESULT],mips);
    mips->put_Inst(new Inst(MIPS_LI, "$v0", "5"));
    mips->put_Inst(new Inst(MIPS_SYSCALL));

    mips->put_Inst(new Inst(MIPS_MOVE, regToStr[num], "$v0"));
    regs[num].dirty = true;   
}

void Block::emit_write_function(Mips * mips, Tac* tac){
    for(auto &it : nameToAddress){
        AddressDesc * adr = it.second;
        if(adr->reg_num == a0){
           if (adr->isPlus) {
                mips->put_Inst(new Inst(MIPS_SW,"$a0","$fp", to_string(adr->offset)));
            } else {
                mips->put_Inst(new Inst(MIPS_SW,"$a0","$fp", to_string(-adr->offset)));
            }
            regs[a0].dirty = false;
            regs[a0].isAllocated = false;
            adr->reg_num = NO_REG;
        }
    }
    mips->put_Inst(new Inst(MIPS_LI,"$v0","1"));
    if(tac->operands[RESULT][0] == '#'){
        mips->put_Inst(new Inst(MIPS_LI,"$a0",tac->operands[RESULT].substr(1)));
    }else{
        Register reg = getRegister(tac->operands[RESULT],mips);
        mips->put_Inst(new Inst(MIPS_MOVE, "$a0", regToStr[reg]));
    }
    mips->put_Inst(new Inst(MIPS_SYSCALL));
    mips->put_Inst(new Inst(MIPS_LI, "$v0", "4"));
    mips->put_Inst(new Inst(MIPS_LA, "$a0", "_eol"));
    mips->put_Inst(new Inst(MIPS_SYSCALL));
}



//JUMP和LABEL
bool is_leader(Tac* tac) {
    if (tac->tac_type == Tac::GOTO
    || tac->tac_type == Tac::IF
    || tac->tac_type == Tac::LABEL || tac->tac_type == Tac::FUNC 
    ) {
        return true;
    }
    return false;
}

// JUMP后一句
bool after_jump(Tac* tac) {
    if (tac->tac_type == Tac::GOTO
    || tac->tac_type == Tac::IF || tac->tac_type == Tac::CALL) { 
        return true;
    }
    return false;
}

void init_block_list() {
    auto itr = tac_vector.begin();
    while(itr != tac_vector.end()){
        if(itr == tac_vector.begin() || (*itr)->tac_type ==  Tac::LABEL || (*itr)->tac_type ==  Tac::FUNC
       ){
            Block *node = new Block(*itr);
            block_list.push_back(node);
        }else if (itr != tac_vector.begin()){
            itr--;
            auto pre = itr;
            itr++;
            if(after_jump(*pre)){
                Block *node = new Block(*itr);
                block_list.push_back(node);
                itr++;
                continue;
            }
            Block *node = block_list.back();
            node->ir.push_back(*itr);
        }
        itr++;

    }
}

void print_blocks() {
    for (auto it : block_list) {
        it->print();
    }
}

void _mips_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void _mips_iprintf(const char *fmt, ...){
    va_list args;
    printf("  "); // `iprintf` stands for indented printf
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void emit_preamble(){
    _mips_printf("# SPL compiler generated assembly");
    _mips_printf(".data");
    _mips_printf("_prmpt: .asciiz \"Enter an integer: \"");
    _mips_printf("_eol: .asciiz \"\\n\"");
    _mips_printf(".globl main");
    _mips_printf(".text");
}


struct VarDesc {    // the variable descriptor
    string var;
    Register reg;
    int offset; // the offset from stack
    /* add other fields as you need */
    struct VarDesc *next;
} *vars;

Mips* gen_mips() {
    Mips *m = new Mips;
    auto iter = block_list.begin();
    while (iter != block_list.end()) {
        Block *block = (*iter);
        block->set_mips(m);
        block->gen_code();
        iter++;
    }
    return m;
}

void emit_code(){
    emit_preamble();
    Mips *mips = gen_mips();
    mips->output();
}

void mips32_gen(){
    regs[zero].name = "$zero";
    regs[at].name = "$at";
    regs[v0].name = "$v0"; regs[v1].name = "$v1";
    regs[a0].name = "$a0"; regs[a1].name = "$a1";
    regs[a2].name = "$a2"; regs[a3].name = "$a3";
    regs[t0].name = "$t0"; regs[t1].name = "$t1";
    regs[t2].name = "$t2"; regs[t3].name = "$t3";
    regs[t4].name = "$t4"; regs[t5].name = "$t5";
    regs[t6].name = "$t6"; regs[t7].name = "$t7";
    regs[s0].name = "$s0"; regs[s1].name = "$s1";
    regs[s2].name = "$s2"; regs[s3].name = "$s3";
    regs[s4].name = "$s4"; regs[s5].name = "$s5";
    regs[s6].name = "$s6"; regs[s7].name = "$s7";
    regs[t8].name = "$t8"; regs[t9].name = "$t9";
    regs[k0].name = "$k0"; regs[k1].name = "$k1";
    regs[gp].name = "$gp";
    regs[sp].name = "$sp"; regs[fp].name = "$fp";
    regs[ra].name = "$ra";
    vars = (struct VarDesc*)malloc(sizeof(struct VarDesc));
    vars->next = NULL;
    init_block_list();
    emit_code();
}

#endif // MIPS_H