#include <ir.hh>
#include <lowerir.hh>
#include <program.hh>
#include <symbol.hh>
#include <utils.hh>
static IR_VReg *imme2reg(Func *func, Basic_Block_Node *bb, IR_Operation *op, std::list<IR_Instr *>::iterator it) {
    if (op->get_op_kind() == reg) return op->get_vreg();
    IR_VReg *new_reg = new IR_VReg(op->get_type());
    func->add_local_vreg(new_reg);
    IR_Instr *assign = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, new_reg, op->copy()));
    bb->get_info().add_instr_prev(assign, it);
    op->reset_vreg(new_reg);
    return new_reg;
}
void lowerir_func(Func *func, BackEnd *backend) {
    for (auto &bb : func->get_basic_blocks()) {
        std::list<IR_Instr *> &instr_list = bb->get_info().get_instr_list();
        auto it = instr_list.begin();
        while (it != instr_list.end()) {
            IR_Instr *instr = *it;
            switch (instr->get_instr_type()) {
            case IR_ALLOCA:
                break;
            case IR_GEP: {
                IR_Operation *gep_src = instr->gep_instr.get_src()->copy();
                IR_VReg *gep_dest = instr->gep_instr.get_dest();
                IR_Operation *idx1 = instr->gep_instr.get_idx1();
                IR_VReg *dest1 = new IR_VReg(new Type(I32));
                func->add_local_vreg(dest1);
                IR_Instr *mul1 = new IR_Instr(IR_Binary_Instr(IR_MUL, dest1, idx1->copy(), new IR_Operation(Basic_Value((I32CONST_t) gep_src->get_type()->get_size() << backend->wordsize))));
                bb->get_info().add_instr_prev(mul1, it);
                IR_VReg *offset = dest1;
                if (instr->gep_instr.get_if_idx2()) {
                    IR_VReg *dest2 = new IR_VReg(new Type(I32));
                    func->add_local_vreg(dest2);
                    IR_Operation *idx2 = instr->gep_instr.get_idx2();
                    IR_Instr *mul2 = new IR_Instr(IR_Binary_Instr(IR_MUL, dest2, idx2->copy(), new IR_Operation(Basic_Value((I32CONST_t) gep_src->get_type()->get_next_level()->get_size() << backend->wordsize))));
                    bb->get_info().add_instr_prev(mul2, it);
                    IR_VReg *dest3 = new IR_VReg(new Type(I32));
                    func->add_local_vreg(dest3);
                    IR_Instr *add = new IR_Instr(IR_Binary_Instr(IR_ADD, dest3, new IR_Operation(dest1), new IR_Operation(dest2)));
                    bb->get_info().add_instr_prev(add, it);
                    offset = dest3;
                }
                delete instr;
                IR_Instr *add = new IR_Instr(IR_Binary_Instr(IR_ADD, gep_dest, gep_src, new IR_Operation(offset)));
                bb->get_info().add_instr_prev(add, it);
                it = instr_list.erase(it);
                continue;
            }
            case IR_LOAD:
                break;
            case IR_STORE:
            case IR_BR:
            case IR_CALL:
            case IR_RETURN:
            case IR_BINARY:
                if (instr->binary_instr.get_opcode() == IR_MOD) {
                    IR_VReg *des = instr->binary_instr.get_dest();
                    IR_VReg *src1 = imme2reg(func, bb, instr->binary_instr.get_src1(), it);
                    IR_VReg *src2 = imme2reg(func, bb, instr->binary_instr.get_src2(), it);
                    delete instr;
                    IR_VReg *div_des = des->copy();
                    func->add_local_vreg(div_des);
                    IR_Instr *div = new IR_Instr(IR_Binary_Instr(IR_DIV, div_des, new IR_Operation(src1), new IR_Operation(src2)));
                    bb->get_info().add_instr_prev(div, it);
                    IR_VReg *mul_des = des->copy();
                    func->add_local_vreg(mul_des);
                    IR_Instr *mul = new IR_Instr(IR_Binary_Instr(IR_MUL, mul_des, new IR_Operation(div_des), new IR_Operation(src2)));
                    bb->get_info().add_instr_prev(mul, it);
                    IR_Instr *sub = new IR_Instr(IR_Binary_Instr(IR_SUB, des, new IR_Operation(src1), new IR_Operation(mul_des)));
                    bb->get_info().add_instr_prev(sub, it);
                    it = bb->get_info().get_instr_list().erase(it);
                    continue;
                }
            case IR_UNARY:
                break;
            }
            it++;
        }
    }
}
void lowerir_program(Program *program) {
    assert(program->get_lang_level() == IR);
    for (auto &func : program->get_funcs()) {
        lowerir_func(func, program->get_backend());
    }
    program->set_lang_level(LOWIR);
}