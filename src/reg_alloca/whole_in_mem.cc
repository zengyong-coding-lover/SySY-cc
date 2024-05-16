#include <ir.hh>
#include <reg_alloca/whole_in_mem.hh>
#include <symbol.hh>

void new_store(Func *func, std::map<IR_VReg *, IR_VReg *> &reg_map, size_t &current_reg_id, Basic_Block_Node *node, IR_VReg *dest, std::list<IR_Instr *>::iterator &it) {
    dest->set_reg_id(current_reg_id++);
    IR_VReg *new_dest = dest->copy();
    func->add_local_vreg(new_dest);
    reg_map[dest] = new_dest;
    IR_Instr *new_store = new IR_Instr(IR_Store_Instr(new IR_Operation(dest), new IR_Operation(new_dest)));
    Symbol *symbol = new Symbol("", dest->get_type()->copy(), false, false, 0, false, 0, func, nullptr);
    IR_Instr *new_alloca = new IR_Instr(IR_Alloca_Instr(new_dest, symbol));
    node->get_info().add_instr_prev(new_alloca, it);
    node->get_info().add_instr_prev(new_store, it);
}
void new_load(Func *func, std::map<IR_VReg *, IR_VReg *> &reg_map, size_t &current_reg_id, Basic_Block_Node *node, IR_Operation *src, std::list<IR_Instr *>::iterator &it) {
    if (src->get_op_kind() != reg)
        return;
    if (src->get_vreg()->get_def_type() == instr_def && src->get_vreg()->get_def_instr()->get_instr_type() == IR_ALLOCA)
        return;
    IR_VReg *new_addr = reg_map[src->get_vreg()];
    IR_VReg *new_src = src->get_vreg()->copy();
    func->add_local_vreg(new_src);
    new_src->set_reg_id(current_reg_id++);
    IR_Instr *new_load = new IR_Instr(IR_Load_Instr(new_src, new IR_Operation(new_addr)));
    node->get_info().add_instr_prev(new_load, it);
    src->reset_vreg(new_src);
}
void whole_in_mem_alloca_func(Func *func, BackEnd *backend) {
    if (func->get_is_lib()) return;
    std::map<IR_VReg *, IR_VReg *> reg_map;
    size_t current_reg_id = 0;
    Basic_Block_Node *entry = func->get_entry_bb();
    auto it = entry->get_info().get_instr_list().begin();
    for (auto &param_reg : func->get_param_reg_list()) {
        new_store(func, reg_map, current_reg_id, entry, param_reg, it);
    }
    for (auto &node : func->get_basic_blocks()) {
        current_reg_id = 0;
        if (node != entry)
            it = node->get_info().get_instr_list().begin();
        for (auto &phi : node->get_info().get_phi_list()) {
            new_store(func, reg_map, current_reg_id, node, phi, it);
        }
        IR_VReg *dest;
        IR_Operation *src1, *src2;
        while (it != node->get_info().get_instr_list().end()) {
            current_reg_id = 0;
            IR_Instr *instr = *it;
            dest = nullptr;
            src1 = src2 = nullptr;
            switch (instr->get_instr_type()) {
            case IR_ALLOCA:
                dest = nullptr; //instr->alloca_instr.get_dest();
                break;
            case IR_BINARY:
                dest = instr->binary_instr.get_dest();
                src1 = instr->binary_instr.get_src1();
                src2 = instr->binary_instr.get_src2();
                break;
            case IR_UNARY:
                dest = instr->unary_instr.get_dest();
                src1 = instr->unary_instr.get_src();
                break;
            case IR_LOAD:
                dest = instr->load_instr.get_dest();
                src1 = instr->load_instr.get_addr();
                break;
            case IR_STORE:
                src1 = instr->store_instr.get_addr();
                src2 = instr->store_instr.get_src();
                break;
            case IR_BR:
                src1 = instr->br_instr.get_cond();
                for (auto &pred : node->get_succs()) {
                    for (auto &phi : pred->get_info().get_bb_param()) {
                        new_load(func, reg_map, current_reg_id, node, phi, it);
                    }
                }
                break;
            case IR_RETURN:
                src1 = instr->ret_instr.get_ret_val();
                break;
            case IR_GEP:
                assert(0);
                break;
            case IR_CALL:
                dest = instr->call_instr.get_dest();
                for (auto &param : instr->call_instr.get_args()) {
                    new_load(func, reg_map, current_reg_id, node, param, it);
                }
                break;
            }
            if (src1) {
                new_load(func, reg_map, current_reg_id, node, src1, it);
            }
            if (src2) {
                new_load(func, reg_map, current_reg_id, node, src2, it);
            }
            current_reg_id = 0;
            it++;
            if (dest) {
                new_store(func, reg_map, current_reg_id, node, dest, it);
            }
        }
    }
    func->set_physics_reg_num(backend->reg_num);
}
void whole_in_mem_alloca_program(Program *program) {
    for (auto &func : program->get_funcs()) {
        whole_in_mem_alloca_func(func, program->get_backend());
    }
}