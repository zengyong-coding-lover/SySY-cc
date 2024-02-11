#ifndef __TOY_IR__
#define __TOY_IR__
#include <basic_value.hh>
#include <graph.hh>
#include <symbol.hh>
#include <type.hh>
#include <utils.hh>
typedef enum {
    IR_BINARY,
    IR_UNARY,
    IR_BR,
    IR_RETURN,
    IR_CALL,
    IR_STORE,
    IR_LOAD,
    IR_GEP,
    IR_ALLOCA,
} IR_Instr_Type;
typedef enum {
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_AND,
    IR_OR,
    IR_EQ,
    IR_NE,
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
} IR_Binary_Opcode;
typedef enum {
    IR_NEG,
    IR_NOT,
    // IR_INC,
    // IR_DEC,
    IR_ASSIGN,
} IR_Unary_Opcode;

class IR_Instr;
class Func;
class IR_Operation;
class Basic_Block_Node_Info;
class Basic_Block_Edge_Info;
typedef Graph_Node<Basic_Block_Node_Info, Basic_Block_Edge_Info> Basic_Block_Node;
typedef Graph_Edge<Basic_Block_Node_Info, Basic_Block_Edge_Info> Basic_Block_Edge;
typedef Graph<Basic_Block_Node_Info, Basic_Block_Edge_Info> Basic_Block_Graph;

class Basic_Block_Node_Info {
private:
    std::list<IR_Instr *> instr_list; // 指令列表

    std::vector<IR_VReg *> bb_phi_list; // 基本块参数
    Basic_Block_Node *block_node; // 对应的 block_node
    Func *func; // 基本快所在的函数

public:
    Basic_Block_Node_Info();
    void add_instr_tail(IR_Instr *instr);
    void add_instr_head(IR_Instr *instr);
    std::list<IR_Instr *> &get_instr_list();
    void set_instr_list(std::list<IR_Instr *> instr_list);
    void add_instr_next(IR_Instr *instr, IR_Instr *prev_instr);
    void add_instr_prev(IR_Instr *instr, IR_Instr *next_instr);
    void add_instr_next(IR_Instr *instr, std::list<IR_Instr *>::iterator prev_instr);
    void add_instr_prev(IR_Instr *instr, std::list<IR_Instr *>::iterator next_instr);
    void move_instr_list(std::list<IR_Instr *>::iterator insert_loc, std::list<IR_Instr *> list, std::list<IR_Instr *>::iterator start, std::list<IR_Instr *>::iterator end);
    void delete_instr(IR_Instr *instr);
    std::list<IR_Instr *>::iterator delete_instr(std::list<IR_Instr *>::iterator instr);
    Basic_Block_Node *get_block_node();
    void set_node(Basic_Block_Node *node);
    void add_phi(IR_VReg *phi);
    std::vector<IR_VReg *> &get_phi_list();
    void print();
    void set_func(Func *func);
    Func *get_func();
    ~Basic_Block_Node_Info();
};

class Basic_Block_Edge_Info {
private:
    std::vector<IR_Operation *> bb_param_list; // 基本块参数
    Basic_Block_Edge *bb_edge; // 对应的边

public:
    Basic_Block_Edge_Info() {
    }
    void set_edge(Basic_Block_Edge *edge);
    void add_bb_param(IR_Operation *op);
    std::vector<IR_Operation *> &get_bb_param() {
        return this->bb_param_list;
    }
    void print_bb_params(LANG_LEVEL level);
    ~Basic_Block_Edge_Info();
};

typedef enum {
    instr_def,
    param_def,
    bb_phi_def,
} Def_Type;
class IR_VReg {
private:
    Type *p_type; // 类型
    size_t vreg_id; // 虚拟寄存器编号

    std::vector<IR_Operation *> use_list; // 使用链
    Def_Type def_type; // 定值类型
    union {
        IR_Instr *instr; // 定值的指令
        struct { // func param
            size_t param_id;
            Func *func;
        };
        struct { // bb_phi
            Basic_Block_Node *bb;
            size_t index;
        };
    };

    size_t reg_id; // 物理寄存器编号

public:
    void *extra_info;

    IR_VReg(Type *type, size_t id = 0);
    IR_VReg(Type *type, size_t id, IR_Instr *instructon);
    IR_VReg(Type *type, size_t id, Func *func, size_t param_id);
    IR_VReg *copy();
    void add_use(IR_Operation *use);
    void set_id(size_t id);
    void set_instr_def(IR_Instr *instr);
    void set_bb_phi_def(Basic_Block_Node *bb);
    void set_param_def(Func *func, size_t param_index);
    std::vector<IR_Operation *> get_use_list();
    bool delete_use(IR_Operation *op);
    Type *get_type();
    IR_Instr *get_def_instr();
    Basic_Block_Node *get_def_bb();
    void print(LANG_LEVEL level);
    Def_Type get_def_type();
    size_t get_id() { return vreg_id; }
    void set_reg_id(size_t reg_id) { this->reg_id = reg_id; }
    // void set_offset(int offset) { this->offset = offset; }
    size_t get_reg_id() { return reg_id; }
    // int get_offset() { return offset; }
    ~IR_VReg();
};
typedef enum {
    bb_param_use,
    instr_use,
} Use_Type;
typedef enum {
    reg,
    imme,
    global_imme,
} Op_Kind;
class IR_Operation {
private:
    IR_VReg *p_vreg;
    Symbol *p_global_sym;
    Basic_Value val;
    Op_Kind op_kind;
    // enum {
    //     // bb_param_ptr,
    //     instr_ptr,
    // } used_type;
    Use_Type used_type;
    union {
        Basic_Block_Edge *bb_edge; //bb_param
        IR_Instr *p_instr;
    };

public:
    IR_Operation(Symbol *p_global_sym);
    IR_Operation(IR_VReg *p_reg);
    IR_Operation(Basic_Value val);
    IR_VReg *get_vreg() {
        assert(this->op_kind == reg);
        return p_vreg;
    }
    Symbol *get_sym() {
        assert(this->op_kind == global_imme);
        return p_global_sym;
    }
    Basic_Value get_basic_value() {
        assert(this->op_kind == imme);
        return val;
    }
    void reset_vreg(IR_VReg *p_reg);
    void set_instr_use(IR_Instr *instr_use);
    void set_bb_param_use(Basic_Block_Edge *edge);
    IR_Operation *copy();
    IR_Instr *get_use_instr();
    Basic_Block_Edge *get_use_bb_edge();
    Op_Kind get_op_kind() { return op_kind; }
    Use_Type get_use_type() { return used_type; }
    Type *get_type();
    void print(LANG_LEVEL level);
    ~IR_Operation();
};
class IR_Binary_Instr {
private:
    IR_Binary_Opcode opcode;
    IR_VReg *dest;
    IR_Operation *src1;
    IR_Operation *src2;
    friend IR_Instr;

public:
    IR_Binary_Instr(IR_Binary_Opcode opcode, IR_VReg *dest, IR_Operation *src1, IR_Operation *src2);
    IR_Binary_Opcode get_opcode();
    void print(LANG_LEVEL level);
    IR_VReg *get_dest() { return dest; }
    IR_Operation *get_src1() { return src1; }
    IR_Operation *get_src2() { return src2; }
    void swap_src() { std::swap(src1, src2); }
    // ~IR_Binary_Instr();
};
class IR_Unary_Instr {
private:
    IR_Unary_Opcode opcode;
    IR_VReg *dest;
    IR_Operation *src;
    friend IR_Instr;

public:
    IR_Unary_Instr(IR_Unary_Opcode opcode, IR_VReg *dest, IR_Operation *src);
    void print(LANG_LEVEL level);
    IR_VReg *get_dest() { return dest; }
    IR_Operation *get_src() { return src; }
    IR_Unary_Opcode get_unary_opcode() { return opcode; }
    // ~IR_Unary_Instr();
};
class IR_Store_Instr {
private:
    IR_Operation *src;
    IR_Operation *addr;
    friend IR_Instr;

public:
    IR_Store_Instr(IR_Operation *src, IR_Operation *addr);
    void print(LANG_LEVEL level);
    IR_Operation *get_src() { return src; }
    IR_Operation *get_addr() { return addr; }
    // ~IR_Store_Instr();
};
class IR_Load_Instr {
private:
    IR_VReg *dest;
    IR_Operation *addr;
    friend IR_Instr;

public:
    IR_Load_Instr(IR_VReg *dest, IR_Operation *addr);
    void print(LANG_LEVEL level);
    IR_VReg *get_dest() { return dest; }
    IR_Operation *get_addr() { return addr; }
    // ~IR_Load_Instr();
};

class IR_Call_Instr {
private:
    Func *func;
    std::vector<IR_Operation *> args;
    IR_VReg *dest;
    friend IR_Instr;

public:
    IR_Call_Instr() { }
    IR_Call_Instr(Func *func, IR_VReg *dest);
    void add_arg(IR_Operation *arg);
    std::vector<IR_Operation *> &get_args();
    void print(LANG_LEVEL level);
    IR_VReg *get_dest() { return dest; }
    Func *get_func() { return func; }
    // ~IR_Call_Instr();
};
class IR_Gep_Instr {
private:
    IR_VReg *dest;
    IR_Operation *src;
    IR_Operation *idx1;
    bool if_idx2;
    IR_Operation *idx2;
    friend IR_Instr;

public:
    IR_Gep_Instr(IR_VReg *dest, IR_Operation *src, IR_Operation *idx1);
    IR_Gep_Instr(IR_VReg *dest, IR_Operation *src, IR_Operation *idx1, IR_Operation *idx2);
    void print(LANG_LEVEL level);
    IR_VReg *get_dest() { return dest; }
    bool get_if_idx2() { return if_idx2; }
    IR_Operation *get_src() { return src; }
    IR_Operation *get_idx2() { return idx2; }
    IR_Operation *get_idx1() { return idx1; }

    // ~IR_Gep_Instr();
};

class IR_Alloca_Instr {
private:
    IR_VReg *dest;
    Symbol *p_stack_sym;
    friend IR_Instr;

public:
    IR_Alloca_Instr(IR_VReg *dest, Symbol *p_stack_sym);
    void print(LANG_LEVEL level);
    IR_VReg *get_dest() { return dest; }
    Symbol *get_stack_sym() { return p_stack_sym; }
    // ~IR_Alloca_Instr();
};
class IR_Br_Instr {
private:
    IR_Operation *cond;
    Basic_Block_Edge *true_bb;
    Basic_Block_Edge *false_bb;
    friend IR_Instr;

public:
    IR_Br_Instr(IR_Operation *cond, Basic_Block_Edge *true_bb, Basic_Block_Edge *false_bb);
    IR_Br_Instr(Basic_Block_Edge *true_bb);
    void print(LANG_LEVEL level);
    IR_Operation *get_cond() { return cond; }
    Basic_Block_Edge *get_true_edge() { return true_bb; }
    Basic_Block_Edge *get_false_edge() { return false_bb; }
    void set_true_edge(Basic_Block_Edge *true_bb);
    void set_false_edge(Basic_Block_Edge *false_bb);

    // ~IR_Br_Instr();
};

class IR_Ret_Instr {
private:
    IR_Operation *ret_val;
    friend IR_Instr;

public:
    IR_Ret_Instr(IR_Operation *ret_val);
    void print(LANG_LEVEL level);
    IR_Operation *get_ret_val() { return ret_val; }

    // ~IR_Ret_Instr();
};
class IR_Instr {
private:
    IR_Instr_Type instr_type;
    Basic_Block_Node *basic_block;

public:
    IR_Call_Instr call_instr; //it's ugly, but emm...

    union {
        IR_Binary_Instr binary_instr;
        IR_Unary_Instr unary_instr;
        IR_Store_Instr store_instr;
        IR_Load_Instr load_instr;
        IR_Gep_Instr gep_instr;
        IR_Alloca_Instr alloca_instr;
        IR_Br_Instr br_instr;
        IR_Ret_Instr ret_instr;
    };
    IR_Instr(IR_Binary_Instr binary_instr);
    IR_Instr(IR_Unary_Instr unary_instr);
    IR_Instr(IR_Store_Instr store_instr);
    IR_Instr(IR_Load_Instr load_instr);
    IR_Instr(IR_Call_Instr call_instr);
    IR_Instr(IR_Gep_Instr gep_instr);
    IR_Instr(IR_Alloca_Instr alloca_instr);
    IR_Instr(IR_Br_Instr br_instr);
    IR_Instr(IR_Ret_Instr ret_instr);
    IR_VReg *get_des();
    void call_add_arg(IR_Operation *arg);
    void set_block(Basic_Block_Node *bb);
    Basic_Block_Node *get_block();
    IR_Instr_Type get_instr_type();
    void print(LANG_LEVEL level);
    ~IR_Instr();
};

#endif