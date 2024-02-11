#ifndef __BACKEND_RISCV__
#define __BACKEND_RISCV__
#include "ir.hh"
#include "symbol.hh"
#include <backend/backend.hh>
#include <bits/types/FILE.h>
#include <graph.hh>
#include <string>
#include <utils.hh>
#include <vector>

class Func;
class RISCV : public BackEnd {
public:
    RISCV()
        : BackEnd(RISCV32, 32, 25, 8, 2, 8, 1, 0, 31, 2) {
    }
    REG update_reg_id(REG reg_id) override {
        if (reg_id < 8)
            return 10 + reg_id;
        if (reg_id < 11)
            return 5 + reg_id - 8;
        if (reg_id < 14)
            return 28 + reg_id - 11;
        return 9 + reg_id - 14;
    }
    Arch get_arch() override { return RISCV32; }

    ~RISCV() override { }
};

static std::string get_reg_name(REG reg) {
    return "x" + std::to_string(reg);
}
typedef enum {
    RISCV_ADDI,
    RISCV_ADD,
    RISCV_SUB,
    RISCV_MOV,
    RISCV_LW,
    RISCV_SW,
    RISCV_LI,
    RISCV_LA,
    RISCV_NEG,
    RISCV_BEQ,
    RISCV_BNE,
    RISCV_BGE,
    RISCV_BLE,
    RISCV_BNEZ,
    RISCV_J,

    RISCV_CALL,
    RISCV_JR,
    RISCV_SEQZ,
    RISCV_SNEZ,
    // RISCV_SLTZ,
    RISCV_SGTZ,
    RISCV_SLT,
    RISCV_XORI,

    RISCV_MUL,
    RISCV_DIV,

} Riscv_Instr_Type;
class Riscv_Operation {
private:
    bool is_imme;
    union {
        int imme12;
        REG reg;
    };

public:
    Riscv_Operation() {
    }
    Riscv_Operation(int imme12) {
        this->is_imme = true;
        this->imme12 = imme12;
    }
    Riscv_Operation(REG reg) {
        this->is_imme = false;
        this->imme12 = imme12;
    }
    friend std::ostream &operator<<(std::ostream &os, const Riscv_Operation &op) {
        if (op.is_imme) {
            os << " " << op.imme12;
            return os;
        }
        os << " " << get_reg_name(op.reg);
        return os;
    }
};

class Riscv_Block_Edge_Info;
class Riscv_Block_Node_Info;
typedef Graph_Node<Riscv_Block_Node_Info, Riscv_Block_Edge_Info> Riscv_Block_Node;
typedef Graph_Edge<Riscv_Block_Node_Info, Riscv_Block_Edge_Info> Riscv_Block_Edge;
typedef Graph<Riscv_Block_Node_Info, Riscv_Block_Edge_Info> Riscv_Graph;

class Riscv_Instr {
private:
    REG rs1;  // 源操作数1
    REG rs2; // 源操作数2
    REG rd; // 目的寄存器
    int imme12; // 合法的立即数
    Riscv_Instr_Type type; // 指令类型
    Func *func; // 函数调用的函数
    Riscv_Block_Node *target; // 跳转目标块
    Symbol *global_sym; // 可用于获取全局变量地址 

public:
    Riscv_Instr(Riscv_Instr_Type type, REG rd = 0, REG rs1 = 0, REG rs2 = 0, int imme12 = 0, Func *func = nullptr, Riscv_Block_Node *target = nullptr, Symbol *global_sym = nullptr);
    friend std::ostream &operator<<(std::ostream &os, const Riscv_Instr &instr);
};

class Riscv_Block_Node_Info {
private:
    std::vector<Riscv_Instr *> instr_list;
    Func *func;
    Riscv_Block_Node *node;

public:
    Riscv_Block_Node_Info() { }
    void set_node(Riscv_Block_Node *node) { this->node = node; }
    void set_func(Func *func) { this->func = func; }
    void add_instr_tail(Riscv_Instr *instr) { instr_list.push_back(instr); }
    std::string get_name();
    friend std::ostream &operator<<(std::ostream &os, Riscv_Block_Node_Info &info);
    ~Riscv_Block_Node_Info() {
        for (auto instr : this->instr_list) {
            delete instr;
        }
    }
};
class Riscv_Block_Edge_Info {
private:
    Riscv_Block_Edge *edge;

public:
    void set_edge(Riscv_Block_Edge *edge) { this->edge = edge; }
};

#endif