#include <backend/riscv.hh>
#include <basic_value.hh>
#include <ir.hh>
#include <reg_alloca/after_alloca.hh>
#include <symbol.hh>
#include <utils.hh>
void after_alloca_func(Func *func, BackEnd *backend) {
    if (func->get_is_lib()) return;
    for (auto &reg : func->get_param_reg_list()) {
        REG newreg = backend->update_reg_id(reg->get_reg_id());
        reg->set_reg_id(newreg);
    }
    for (auto &reg : func->get_local_reg_list()) {
        REG newreg = backend->update_reg_id(reg->get_reg_id());
        reg->set_reg_id(newreg);
    }

    IR_VReg *sp1 = new IR_VReg(nullptr);
    sp1->set_reg_id(backend->sp);
    func->add_local_vreg(sp1);

    IR_VReg *sp2 = new IR_VReg(nullptr);
    sp2->set_reg_id(backend->sp);
    func->add_local_vreg(sp2);

    IR_VReg *sp3 = new IR_VReg(nullptr);
    sp3->set_reg_id(backend->sp);
    func->add_local_vreg(sp3);

    IR_VReg *sp4 = new IR_VReg(nullptr);
    sp4->set_reg_id(backend->sp);
    func->add_local_vreg(sp4);

    IR_VReg *fp1 = new IR_VReg(nullptr);
    fp1->set_reg_id(backend->fp);
    func->add_local_vreg(fp1);

    IR_VReg *fp2 = new IR_VReg(nullptr);
    fp2->set_reg_id(backend->fp);
    func->add_local_vreg(fp2);

    IR_VReg *ra1 = new IR_VReg(nullptr);
    ra1->set_reg_id(backend->ra);
    func->add_local_vreg(ra1);

    IR_Instr *sp2_sp1 = nullptr;
    IR_Instr *sp3_sp2 = nullptr;
    IR_Instr *sp4_sp3 = nullptr;
    IR_Instr *store_fp = nullptr;
    IR_Instr *store_ra = nullptr;
    IR_Instr *get_fp = nullptr;
    int offset = 0;
    for (auto &bb : func->get_basic_blocks()) {
        auto it = bb->get_info().get_instr_list().begin();
        while (it != bb->get_info().get_instr_list().end()) {
            IR_Instr *instr = *it;
            if (instr->get_instr_type() == IR_ALLOCA) {
                IR_VReg *dest = instr->alloca_instr.get_dest();
                for (auto &op : dest->get_use_list()) {
                    switch (op->get_use_type()) {
                    case instr_use: {
                        IR_Instr *use_instr = op->get_use_instr();
                        IR_VReg *new_addr = dest->copy();
                        func->add_local_vreg(new_addr);
                        new_addr->set_reg_id(backend->tmp);

                        IR_Instr *get_addr = new IR_Instr(IR_Binary_Instr(IR_ADD, new_addr, new IR_Operation(sp4), new IR_Operation(I32CONST_t(offset))));
                        use_instr->get_block()->get_info().add_instr_prev(get_addr, use_instr);
                        op->reset_vreg(new_addr);
                        break;
                    }
                    case bb_param_use:
                        assert(0);
                    }
                }

                size_t size = instr->alloca_instr.get_stack_sym()->get_type()->get_size() << backend->wordsize;
                delete instr;
                // IR_Instr *add = new IR_Instr(IR_Binary_Instr(IR_ADD, dest, new IR_Operation(sp2), new IR_Operation(Basic_Value(I32CONST_t(offset)))));

                offset += size;
                // bb->get_info().add_instr_prev(add, it);
                it = bb->get_info().get_instr_list().erase(it);
                continue;
            }
            if (instr->get_instr_type() == IR_CALL) {
                it++;
                continue;
            }
            if (instr->get_instr_type() == IR_BR) {
                it++;
                continue;
            }
            if (instr->get_instr_type() == IR_RETURN) {
                sp2_sp1 = new IR_Instr(IR_Binary_Instr(IR_ADD, sp2, new IR_Operation(sp1), new IR_Operation(Basic_Value(I32CONST_t(-1 << backend->wordsize)))));
                store_fp = new IR_Instr(IR_Store_Instr(new IR_Operation(fp1), new IR_Operation(sp2)));
                sp3_sp2 = new IR_Instr(IR_Binary_Instr(IR_ADD, sp3, new IR_Operation(sp2), new IR_Operation(Basic_Value(I32CONST_t(-1 << backend->wordsize)))));
                store_ra = new IR_Instr(IR_Store_Instr(new IR_Operation(ra1), new IR_Operation(sp3)));
                sp4_sp3 = new IR_Instr(IR_Binary_Instr(IR_ADD, sp4, new IR_Operation(sp3), new IR_Operation(Basic_Value(I32CONST_t(-offset)))));
                get_fp = new IR_Instr(IR_Binary_Instr(IR_ADD, fp2, new IR_Operation(sp4), new IR_Operation(Basic_Value(I32CONST_t(offset + (2 << backend->wordsize))))));

                IR_VReg *new_fp = new IR_VReg(nullptr);
                new_fp->set_reg_id(backend->fp);
                func->add_local_vreg(new_fp);

                IR_VReg *new_ra = new IR_VReg(nullptr);
                new_ra->set_reg_id(backend->ra);
                func->add_local_vreg(new_ra);

                IR_VReg *new_sp1 = new IR_VReg(nullptr);
                new_sp1->set_reg_id(backend->sp);
                func->add_local_vreg(new_sp1);

                IR_VReg *new_sp2 = new IR_VReg(nullptr);
                new_sp2->set_reg_id(backend->sp);
                func->add_local_vreg(new_sp2);

                IR_VReg *new_sp3 = new IR_VReg(nullptr);
                new_sp3->set_reg_id(backend->sp);
                func->add_local_vreg(new_sp3);

                IR_Instr *newsp1_sp4 = new IR_Instr(IR_Binary_Instr(IR_ADD, new_sp1, new IR_Operation(sp4), new IR_Operation(Basic_Value(I32CONST_t(offset)))));
                bb->get_info().add_instr_prev(newsp1_sp4, it);
                IR_Instr *restore_ra = new IR_Instr(IR_Load_Instr(new_ra, new IR_Operation(new_sp1)));
                bb->get_info().add_instr_prev(restore_ra, it);
                IR_Instr *newsp2_newsp1 = new IR_Instr(IR_Binary_Instr(IR_ADD, new_sp2, new IR_Operation(new_sp1), new IR_Operation(Basic_Value(I32CONST_t(1 << backend->wordsize)))));
                bb->get_info().add_instr_prev(newsp2_newsp1, it);
                IR_Instr *restore_fp = new IR_Instr(IR_Load_Instr(new_fp, new IR_Operation(new_sp2)));
                bb->get_info().add_instr_prev(restore_fp, it);
                IR_Instr *newsp3_newsp2 = new IR_Instr(IR_Binary_Instr(IR_ADD, new_sp3, new IR_Operation(new_sp2), new IR_Operation(Basic_Value(I32CONST_t(1 << backend->wordsize)))));
                bb->get_info().add_instr_prev(newsp3_newsp2, it);
            }
            it++;
        }
    }
    func->get_entry_bb()->get_info().add_instr_head(get_fp);
    func->get_entry_bb()->get_info().add_instr_head(sp4_sp3);
    func->get_entry_bb()->get_info().add_instr_head(store_ra);
    func->get_entry_bb()->get_info().add_instr_head(sp3_sp2);
    func->get_entry_bb()->get_info().add_instr_head(store_fp);
    func->get_entry_bb()->get_info().add_instr_head(sp2_sp1);
}

void after_alloca_program(Program *program) {
    BackEnd *backend = new RISCV();
    for (auto &func : program->get_funcs()) {
        after_alloca_func(func, backend);
    }
    delete backend;
}
