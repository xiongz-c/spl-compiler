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

class Inst {
public:
    string res;
    string arg1;
    string arg2;
    MipsOp op;

    stirng debug_info; // comment for debug


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

    }

    void set_debug(const string &debug_info) {
        this->debug_info = debug_info;
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
};


class Block {
public:
    vector<Tac*> ir;

    Block(Tac* ld) {
        this.ir.push_back(ld);
    }

    void print() {

        for (auto item: ir) {
            cout << item->to_string()
        }
        cout << endl << "========block========" << endl;
    }
};

list<Block> block_list;

bool is_leader(Tac* tac) {
    if (tac->tac_type == Tac::GOTO
    || tac->tac_type == Tac::IF
    || tac->tac_type == Tac::LABEL) { // 跳转目标一定是label
        return true;
    }

    return false;
}

void init_block_list() {
    auto itr = tac_vector.begin();
    while(itr != tac_vector.end()){
        if (itr == tac_vector.begin() || is_leader(*itr) || is_leader(*(itr-1))) {
            Block *node = new Block(*itr);
            block_list.push_back(node);
        } else {
            Block *node = block_list.back();
            node->ir.push_back(*itr);
        }
    }
}

void print_blocks() {
    for (auto it : block_list) {
        it.print();
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


typedef enum {
    zero, at, v0, v1, a0, a1, a2, a3,
    t0, t1, t2, t3, t4, t5, t6, t7,
    s0, s1, s2, s3, s4, s5, s6, s7,
    t8, t9, k0, k1, gp, sp, fp, ra, NUM_REGS
} Register;


struct RegDesc {    // the register descriptor
    string name;
    string var;
    bool dirty; // value updated but not stored
    /* add other fields as you need */
} regs[NUM_REGS];


struct VarDesc {    // the variable descriptor
    string var;
    Register reg;
    int offset; // the offset from stack
    /* add other fields as you need */
    struct VarDesc *next;
} *vars;


void emit_code(){
    emit_preamble();
    emit_read_function();
    emit_write_function();
    auto itr = tac_vector.begin();
    while(itr != tac_vector.end()){
        Tac * tac =  *itr;
        //tac->to_string();
        itr++;
    }
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
    print_blocks();
    emit_code();
}

#endif // MIPS_H