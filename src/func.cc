#include <ast_tree.hh>
#include <cstddef>
#include <fstream>
#include <func.hh>
#include <ir.hh>
#include <ostream>
#include <program.hh>
#include <symbol.hh>
#include <type.hh>
#include <utils.hh>
Func::Func(std::string name, bool is_void, bool is_lib) {
    this->name = name;
    this->is_void = is_void;
    this->block_graph = new Basic_Block_Graph();
    this->is_lib = is_lib;
    this->ast_body = nullptr;
    this->riscv_graph = nullptr;
    this->physics_reg_num = -1;
}
Func::Func(std::string name, Type *ret_type, bool is_lib) {
    this->name = name;
    this->ret_type = ret_type;
    this->is_lib = is_lib;
    this->is_void = false;
    this->block_graph = new Basic_Block_Graph();
    this->ast_body = nullptr;
    this->riscv_graph = nullptr;
    this->physics_reg_num = -1;
}
void Func::set_physics_reg_num(size_t num) {
    this->physics_reg_num = num;
}
size_t Func::get_physics_reg_num() const {
    return this->physics_reg_num;
}
void Func::set_ret_type(Type *ret_type) {
    assert(!this->ret_type);
    this->ret_type = ret_type;
}
void Func::set_name(std::string name) {
    this->name = name;
}
void Func::set_is_void(bool is_void) {
    this->is_void = is_void;
}
void Func::set_ast_body(Ast_Node *ast_body) {
    this->ast_body = ast_body;
}
Ast_Node *Func::get_ast_body() {
    return this->ast_body;
}
std::list<Basic_Block_Node *> &Func::get_basic_blocks() {
    return this->block_graph->get_nodes();
}
void Func::set_ir_body(Basic_Block_Graph *ir_body) {
    this->block_graph = ir_body;
}
void Func::add_symbol(Symbol *symbol) {
    this->locals.push_back(symbol);
}
void Func::add_param(Symbol *symbol) {
    this->params.push_back(symbol);
}
std::vector<Symbol *> &Func::get_symbols() {
    return this->locals;
}
std::vector<Symbol *> &Func::get_params() {
    return this->params;
}
Type *Func::get_ret_type() {
    return this->ret_type;
}
bool Func::get_is_void() {
    return this->is_void;
}
std::string Func::get_name() {
    return this->name;
}

void Func::set_entry_bb(Basic_Block_Node *bb) {
    this->entry_bb = bb;
}
Basic_Block_Node *Func::get_entry_bb() {
    return this->entry_bb;
}
void Func::set_exit_bb(Basic_Block_Node *bb) {
    this->exit_bb = bb;
}
Basic_Block_Node *Func::get_exit_bb() {
    return this->exit_bb;
}
void Func::add_param_vreg(IR_VReg *vreg) {
    vreg->set_param_def(this, this->param_reg_list.size());
    this->param_reg_list.push_back(vreg);
}
void Func::add_local_vreg(IR_VReg *vreg) {
    this->local_reg_list.push_back(vreg);
}
void Func::print_symbols(LANG_LEVEL level) {
    for (auto param : this->params) {
        param->print(level);
    }
    for (auto sym : this->locals) {
        sym->print(level);
    }
}
void Func::print(LANG_LEVEL level) {
    if (level == AST) {
        std::cout << "name:" << this->name;
        std::cout << "     type:func  ";

        std::cout << "    ret_type:";
        if (this->is_void)
            std::cout << " void";
        else {
            this->ret_type->print();
        }
        std::cout << std::endl;
        for (auto param : this->params) {
            std::cout << "          |";
            param->print(AST);
            std::cout << std::endl;
        }
        for (auto sym : this->locals) {
            std::cout << "          |";
            sym->print(AST);
            std::cout << std::endl;
        }
        return;
    }
    if (level == IR) {
        this->reset_block_id();
        this->reset_vreg_id();
        std::cout << "define";
        if (this->is_void)
            std::cout << " void";
        else {
            this->ret_type->print();
        }
        std::cout << " @" << this->name << "(";
        for (size_t i = 0; i < this->param_reg_list.size(); i++) {
            if (i != 0)
                std::cout << ", ";
            this->param_reg_list[i]->print(IR);
        }
        if (this->is_lib)
            std::cout << ");" << std::endl;
        else {
            std::cout << ") {" << std::endl;
            this->block_graph->print_list();
            std::cout << "}" << std::endl
                      << std::endl;
        }
        return;
    }
    if (level == LOWIR) {
        if (is_lib) return;
        std::cout << "define";
        if (this->is_void)
            std::cout << " void";
        else {
            this->ret_type->print();
        }
        std::cout << " @" << this->name << "(";
        for (size_t i = 0; i < this->param_reg_list.size(); i++) {
            if (i != 0)
                std::cout << ", ";
            this->param_reg_list[i]->print(LOWIR);
        }
        if (this->is_lib)
            std::cout << ");" << std::endl;
        else {
            std::cout << ") {" << std::endl;
            this->block_graph->print_list();
            std::cout << "}" << std::endl
                      << std::endl;
        }
        return;
    }
}
Basic_Block_Graph *Func::get_block_graph() {
    return this->block_graph;
}
void Func::add_basic_block_tail(Basic_Block_Node *bb) {
    this->block_graph->add_node_tail(bb);
}
void Func::add_basic_block_head(Basic_Block_Node *bb) {
    this->block_graph->add_node_head(bb);
}
void Func::reset_block_id() {
    this->block_graph->reset_id();
}
void Func::reset_vreg_id() {
    size_t reg_cnt = 0;
    for (auto &reg : this->param_reg_list) {
        reg->set_id(reg_cnt++);
    }
    for (auto block : this->block_graph->get_nodes()) {
        for (auto &phi : block->get_info().get_phi_list())
            phi->set_id(reg_cnt++);
        for (auto &instr : block->get_info().get_instr_list()) {
            IR_VReg *des = instr->get_des();
            if (des)
                des->set_id(reg_cnt++);
        }
    }
}
void Func::set_is_lib(bool is_lib) {
    this->is_lib = is_lib;
}
bool Func::get_is_lib() {
    return is_lib;
}
// void Func::set_seqence_vreg_id() {
//     size_t i = 0;
//     for (auto &reg : this->param_reg_list)
//         reg->set_id(i++);
//     for (auto &reg : this->local_reg_list)
//         reg->set_id(i++);
// }
// IR_VReg *Func::get_vreg_by_seqence_vreg_id(size_t index) {
//     if (index < param_reg_list.size())
//         return param_reg_list[index];
//     return local_reg_list[index - param_reg_list.size()]
// }
std::vector<IR_VReg *> &Func::get_param_reg_list() {
    return param_reg_list;
}
std::list<IR_VReg *> &Func::get_local_reg_list() {
    return local_reg_list;
}
void Func::set_program(Program *program) {
    this->program = program;
}
Program *Func::get_program() {
    return this->program;
}
void Func::add_call_instr(IR_Instr *instr) {
    this->call_instrs.push_back(instr);
}
std::vector<IR_Instr *> &Func::get_call_instrs() {
    return call_instrs;
}
void Func::clear_unused_vregs() {
    auto iter = this->local_reg_list.begin();
    while (iter != this->local_reg_list.end()) {
        switch ((*iter)->get_def_type()) {
        case instr_def:
            if (!(*iter)->get_def_instr()) {
                delete *iter;
                iter = this->local_reg_list.erase(iter);
            }
            else
                iter++;
            break;
        case bb_phi_def:
            if (!(*iter)->get_def_bb()) {
                delete *iter;
                iter = this->local_reg_list.erase(iter);
            }
            else
                iter++;
            break;
        case param_def:
            iter++;
            break;
        default:
            iter = this->local_reg_list.erase(iter);
        }
    }
    reset_vreg_id();
}
void Func::output_assembler(std::ofstream &os) {
    if (is_lib) return;
    this->riscv_graph->reset_id();
    os << ".text" << std::endl;
    os << ".align 2" << std::endl;
    os << ".globl " << name << std::endl;
    os << ".type " << name << ", @function " << std::endl;
    os << name << ":" << std::endl;
    for (auto &block : this->riscv_graph->get_nodes()) {
        os << block->get_info();
    }
    os << std::endl;
}

Func::~Func() {
    delete block_graph;
    for (auto param : this->params)
        delete param;
    for (auto symbol : this->locals)
        delete symbol;
    for (auto reg : this->param_reg_list)
        delete reg;
    for (auto reg : this->local_reg_list)
        delete reg;
    delete riscv_graph;
    if (!this->is_void)
        delete ret_type;
}
