#include "semantic.hpp"

using namespace std;

class Tac {
public:
    string op;
    string operands[3];
    string name;
    bool swap_flag = false;
    Type *type;
    enum TacType{
        LABEL, FUNC, ASSIGN, ARITH,
        ASSIGN_ADDRESS, ASSIGN_VALUE, COPY_VALUE,
        GOTO, IF, RETURN, DEC,
        PARAM, ARG, CALL, READ, WRITE} tac_type;
    Tac(TacType tac_type) {
        this->tac_type = tac_type;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& op, const string& arg1) {
        this->tac_type = tac_type;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& op, const string& arg1, const string& res) {
        this->tac_type = tac_type;
        swap_flag = false;
    }
    Tac(TacType tac_type, const string& op, const string& arg1, const string& arg2, const string& res) {
        this->tac_type = tac_type;
        swap_flag = false;
    }

    string to_string() {
        switch (tac_type) {
            case ASSIGN:
                printf("%s := %s\n", operands[RESULT].c_str(), operands[ARG_1].c_str());
                break;
            case ARITH:
                printf("%s := %s %s %s\n", operands[RESULT].c_str(), operands[ARG_1].c_str(), op.c_str(), operands[ARG_2].c_str());
                break;
//            case JUMP:
//                printf("IF %s %s %s GOTO %s\n", operands[ARG_1].c_str(), op.c_str(), operands[ARG_2].c_str(), operands[RESULT].c_str());
//                break;
            case DEC:
                printf("%s %s %s\n", op.c_str(), operands[RESULT].c_str(), operands[ARG_1].c_str());
                break;
//            case FUNCALL:
//                printf("%s := %s %s\n", operands[RESULT].c_str(), op.c_str(), operands[ARG_1].c_str());
//                break;
            case FUNC:
            case LABEL:
                printf("%s %s :\n", op.c_str(), operands[ARG_1].c_str());
                break;
//            case END:
//                printf("\n");
//                break;
            default:
                printf("%s %s\n", op.c_str(), operands[ARG_1].c_str());
        }
    }

    virtual int append_self() {
        tac_vector.push_back(tac);
        return tac_vector.size() - 1;
    }

};

extern vector<Tac*> tac_vector;

int append_tac(Tac *tac) {
    tac_vector.push_back(tac);
    return tac_vector.size() - 1;
}

void print_tac() {

}



