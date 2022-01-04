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
        {zero,"zero"},
        {at,"at"},
        {v0,"v0"},{v1,"v1"},
        {a0,"a0"}, {a1,"a1"}, {a2,"a2"}, {a3,"a3"},
        {t0,"t0"}, {t1,"t1"}, {t2,"t2"}, {t3,"t3"}, {t4,"t4"}, {t5,"t5"}, {t6,"t6"}, {t7,"t7"},
        {s0,"s0"}, {s1,"s1"}, {s2,"s2"}, {s3,"s3"}, {s4,"s4"}, {s5,"s5"}, {s6,"s6"}, {s7,"s7"},
        {t8,"t8"}, {t9,"t9"}, {k0,"k0"}, {k1,"k1"}, {gp,"gp"}, {sp,"sp"}, {fp,"fp"}, {ra,"ra"},
};

Register localAllocate();

struct AddressDesc{
    string name;
    Register reg_num; // the number of register eg:  t0 is 8
    int offset; // offset to $fp
    bool isPlus;

    AddressDesc(string name, Register reg, int offset, bool isPlus) {
        this->name = name;
        this->reg_num = reg;
        this->offset = offset;
        this->isPlus = isPlus;
    }

};

unordered_map<string, AddressDesc*> nameToAddress;

struct RegDesc {    // the register descriptor
    string name;
    string var;
    bool dirty; // value updated but not stored
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
};

class Block {
public:
    vector<Tac*> ir;
    vector<Tac*> param_tac_list; //如果block是一个函数，需要计算参数的个数，确定fp的offset
    Mips* mips;

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

    // 向当前所属的mips类添加指令
    void gen_code() {
        //this->print();
        this->analysis();
        auto itr = ir.begin();
        while(itr != ir.end()){
            Tac* tac = *(itr);
            if(tac->tac_type == Tac::FUNC){
                generateFunction(this->mips,tac);
            }else if(tac->tac_type == Tac::ASSIGN){
                generateAssign(this->mips,tac);
            }
            itr++;
        }
    }

    void analysis(){
        auto itor = ir.end();
        while(itor != ir.begin()){
            itor--;
            Tac * tac = *(itor);
            if(tac->tac_type == Tac::PARAM){
                param_tac_list.push_back(tac);
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


void Block::generateFunction(Mips* mips, Tac* tac){
    Inst * ins = new Inst(MIPS_LABEL,tac->operands[RESULT].c_str());
    mips->put_Inst(ins);
    mips->put_Inst(new Inst(MIPS_MOVE, "$fp", "$sp"));
    int param_size = this->param_tac_list.size();
    int offset_cnt = param_size + 2;
    for (auto item = this->param_tac_list.begin(); item != this->param_tac_list.end() ; ++item) {
        Register tmp = argAllocate();
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

void Block::generateAssign(Mips* mips, Tac* tac){
    cout << tac->operands[RESULT] << " is assign " << endl;
    cout << tac->operands[ARG1] << " to be assigned " << endl;
}

//JUMP和LABEL
bool is_leader(Tac* tac) {
    if (tac->tac_type == Tac::GOTO
    || tac->tac_type == Tac::IF
    || tac->tac_type == Tac::LABEL) {
        return true;
    }
    return false;
}

// JUMP后一句
bool after_jump(Tac* tac) {
    if (tac->tac_type == Tac::GOTO
    || tac->tac_type == Tac::IF) { 
        return true;
    }
    return false;
}

void init_block_list() {
    auto itr = tac_vector.begin();
    while(itr != tac_vector.end()){
        if (itr == tac_vector.begin() || is_leader(*itr) || after_jump(*(itr-1))) {
            Block *node = new Block(*itr);
            block_list.push_back(node);
        } else {
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

void emit_read_function(){
    _mips_printf("read:");
    _mips_iprintf("li $v0, 4");
    _mips_iprintf("la $a0, _prmpt");
    _mips_iprintf("syscall");
    _mips_iprintf("li $v0, 5");
    _mips_iprintf("syscall");
    _mips_iprintf("jr $ra");
}

void emit_write_function(){
    _mips_printf("write:");
    _mips_iprintf("li $v0, 1");
    _mips_iprintf("syscall");
    _mips_iprintf("li $v0, 4");
    _mips_iprintf("la $a0, _eol");
    _mips_iprintf("syscall");
    _mips_iprintf("move $v0, $0");
    _mips_iprintf("jr $ra");
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
     emit_read_function();
     emit_write_function();
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