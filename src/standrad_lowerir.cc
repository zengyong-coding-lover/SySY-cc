#include <standrad_lowir.hh>

static IR_VReg *imme2reg(Func *func, Basic_Block_Node *bb, IR_Operation *op, std::list<IR_Instr *>::iterator it) {
    if (op->get_op_kind() == reg) return op->get_vreg();
    IR_VReg *new_reg = new IR_VReg(op->get_type()->copy());
    func->add_local_vreg(new_reg);
    IR_Instr *assign = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, new_reg, op->copy()));
    bb->get_info().add_instr_prev(assign, it);
    op->reset_vreg(new_reg);
    return new_reg;
}

void standrad_lowir_func(Func *func) {
    for (auto block : func->get_basic_blocks()) {
        auto it = block->get_info().get_instr_list().begin();
        while (it != block->get_info().get_instr_list().end()) {
            IR_Instr *instr = *it;
            switch (instr->get_instr_type()) {
            case IR_ALLOCA:
                break;
            case IR_GEP:
                assert(0);
                break;
            case IR_LOAD:
                imme2reg(func, block, instr->load_instr.get_addr(), it);
                break;
            case IR_STORE:
                imme2reg(func, block, instr->store_instr.get_addr(), it);
                imme2reg(func, block, instr->store_instr.get_src(), it);
                break;
            case IR_BR:
                break;
            case IR_BINARY: {
                IR_Operation *src1 = instr->binary_instr.get_src1();
                IR_Operation *src2 = instr->binary_instr.get_src2();
                if (instr->binary_instr.get_opcode() == IR_ADD) {
                    if (src1->get_op_kind() == imme && src2->get_op_kind() == reg) {
                        instr->binary_instr.swap_src();
                        break;
                    }
                }
                imme2reg(func, block, src1, it);
                imme2reg(func, block, src2, it);
                break;
            }
            case IR_CALL:
                break;
            case IR_RETURN:
                break;
            case IR_UNARY:
                break;
            }
            it++;
        }
    }
}
void standrad_lowir_program(Program *program) {
    assert(program->get_lang_level() == LOWIR);
    for (auto func : program->get_funcs()) {
        standrad_lowir_func(func);
    }
}