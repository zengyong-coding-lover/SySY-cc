#ifndef __FUNC__
#define __FUNC__
#include <backend/riscv.hh>
#include <ir.hh>
#include <program.hh>
#include <utils.hh>
class Symbol;
class Ast_Node;
class Type;
class IR_VReg;
class Func {
private:
    std::vector<Symbol *> locals; // 局部变量
    std::vector<Symbol *> params; // 参数
    bool is_void; // 返回值是否是 void
    Type *ret_type; // 返回类型
    std::string name; // 函数名

    // 不同形式的函数体
    Ast_Node *ast_body; // ast
    Basic_Block_Graph *block_graph; // ir
    Riscv_Graph *riscv_graph; // asm

    Basic_Block_Node *entry_bb; //入口块
    Basic_Block_Node *exit_bb; // 出口块
    std::vector<IR_Instr *> call_instrs; // 调用的函数

    std::vector<IR_VReg *> param_reg_list; // ir vreg 形式的形参
    std::list<IR_VReg *> local_reg_list; // 所有本地 reg

    Program *program; // 所在的程序
    bool is_lib; // 是否是外部链接库

    size_t physics_reg_num; // 物理寄存器数量

public:
    Func(std::string name, bool is_void, bool is_lib = false);
    Func(std::string name, Type *ret_type, bool is_lib = false);
    size_t get_physics_reg_num() const;
    void set_physics_reg_num(size_t num);

    void set_ret_type(Type *ret_type);
    void set_name(std::string name);
    void set_is_void(bool is_void);
    void set_ast_body(Ast_Node *ast_body);
    Ast_Node *get_ast_body();
    std::list<Basic_Block_Node *> &get_basic_blocks();
    void set_ir_body(Basic_Block_Graph *ir_body);
    void add_symbol(Symbol *symbol);
    void add_param(Symbol *symbol);
    void add_basic_block_tail(Basic_Block_Node *bb);
    void add_basic_block_head(Basic_Block_Node *bb);
    std::vector<Symbol *> &get_symbols();
    std::vector<Symbol *> &get_params();
    Type *get_ret_type();
    bool get_is_void();
    Basic_Block_Graph *get_block_graph();
    std::string get_name();

    void set_entry_bb(Basic_Block_Node *bb);
    Basic_Block_Node *get_entry_bb();
    void set_exit_bb(Basic_Block_Node *bb);
    Basic_Block_Node *get_exit_bb();

    void add_param_vreg(IR_VReg *vreg);
    void add_local_vreg(IR_VReg *vreg);
    void reset_block_id();
    void reset_vreg_id();

    void set_is_lib(bool is_lib);
    bool get_is_lib();
    // void set_seqence_vreg_id();
    // IR_VReg *get_vreg_by_seqence_vreg_id(size_t index);
    // void set_local_vreg_id();
    std::vector<IR_VReg *> &get_param_reg_list();
    std::list<IR_VReg *> &get_local_reg_list();

    void set_program(Program *program);
    Program *get_program();
    void add_call_instr(IR_Instr *instr);
    std::vector<IR_Instr *> &get_call_instrs();
    void output_assembler(std::ofstream &os);
    void clear_unused_vregs();
    void print_symbols(LANG_LEVEL level);
    void print(LANG_LEVEL level);
    ~Func();
    size_t get_vreg_num() { return this->param_reg_list.size() + this->local_reg_list.size(); }
    void set_riscv_graph(Riscv_Graph *graph) { this->riscv_graph = graph; }
    Riscv_Graph *get_riscv_graph() { return this->riscv_graph; }
};

#endif