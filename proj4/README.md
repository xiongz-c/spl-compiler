# CS323 Project4

> Yifei Li 11811905
>
> Zhuochen Xiong 11811806

## Introduction

Here is the last step for a working compiler: Target Code Generation. The three-address code for SPL language is a kind of mid-level intermediate representation(IR). We need generate executable code (MIPS32 machine) by TAC.

In this project, we are free to choose to start with the SPL language to complete the complete compilation process or choose to start with the standard IR language given by the teacher and only complete the translation process from IR to MIPS assembly. In this project, we are free to choose to start with the SPL language to complete the complete compilation process or choose to start with the standard IR language given by the teacher and only complete the translation process from IR to MIPS assembly.  In addition, we have done some optimizations in the intermediate code generation to get a more streamlined IR. These optimizations are also reflected in the final assembly generation stage, helping to generate leaner assembly code. 

## Design & Implement

### Instruction Represent

```c pp
class Inst {
public:
    string res;
    string arg1;
    string arg2;
    MipsOp op;

    string debug_info; // comment for debug
    Inst(MipsOp op);
    Inst(MipsOp op, const string &res);
    Inst(MipsOp op, const string &res, const string &arg1);
    Inst(MipsOp op, const string &res, const string &arg1, const string &arg2);
    string to_string();
    void print_inst();
    void set_debug(const string &debug_info);
};
```

In the design of the instruction structure, we use a TAC-like design. Among them, we use enumeration types to represent specific operators, and add comments that do not affect actual execution for debugging.

### Basic Block

#### Structure

```cpp
class Block {
public:
    vector<Tac*> ir;
    vector<Tac*> param_tac_list; //if block is function，need calculate param cnt，to set offset
    vector<Tac*> arg_tac_list; 
    Mips* mips;
    bool isStored = false;
    Block(Tac* ld) {this->ir.push_back(ld);}
    // print ir list, for debug
    void print() {
        ...
    }
    void set_mips(Mips* m){
        ...
    }
    // add instruction into mips
    void gen_code() {
        ...
    }
	// reg analysis
    void analysis(){
       ...
    }
};
```

We implement our main code generation method in the block structure. In the divided basic block, we only need to manage the registers available in this basic block and perform corresponding operations.

#### Divide basic blocks

```cpp
// JUMP & LABEL
bool is_leader(Tac* tac) {
    if (tac->tac_type == Tac::GOTO
    || tac->tac_type == Tac::IF
    || tac->tac_type == Tac::LABEL) {
        return true;
    }
    return false;
}

// after JUMP
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
```

We split the basic blocks of IR by electing leaders. As shown in the code, we consider GOTO, IF, LABEL statements as a leader. Also, the IR statement immediately following GOTO and IF is considered the leader of a basic block.

#### Generate callee

```cpp
    // callee generate
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
```

The above methods are our methods of generating callee. The implementation of these functions is based on the generation rules of mips instructions, and the specific implementation will not be repeated.