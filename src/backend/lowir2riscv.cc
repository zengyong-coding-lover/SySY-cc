#include <backend/arch.hh>
#include <backend/backend.hh>
#include <backend/lowir2riscv.hh>
#include <backend/riscv.hh>
#include <func.hh>
#include <utils.hh>
// int offset = 0;
void lowir2riscv_func(Func *func, BackEnd *backend) {
    if (func->get_is_lib()) return;
    Riscv_Graph *riscv_graph = new Riscv_Graph;
    func->set_riscv_graph(riscv_graph);

    std::map<Basic_Block_Node *, Riscv_Block_Node *> block_map;
    for (auto &block : func->get_basic_blocks()) {
        block_map[block] = new Riscv_Block_Node();
    }
    for (auto &block : func->get_basic_blocks()) {
        Riscv_Block_Node *riscv_node = block_map[block];
        riscv_graph->add_node_tail(riscv_node);
        riscv_node->get_info().set_func(func);

        Riscv_Instr *riscv_instr = nullptr;
        for (auto &instr : block->get_info().get_instr_list()) {
            switch (instr->get_instr_type()) {
            case IR_ALLOCA:
                assert(0);
                break;
            case IR_BINARY: {
                REG rd = instr->binary_instr.get_dest()->get_reg_id();
                IR_Operation *src1 = instr->binary_instr.get_src1();
                IR_Operation *src2 = instr->binary_instr.get_src2();
                REG rs1 = src1->get_vreg()->get_reg_id();
                switch (instr->binary_instr.get_opcode()) {
                case IR_ADD:
                    if (src2->get_op_kind() == reg) {
                        riscv_instr = new Riscv_Instr(RISCV_ADD, rd, rs1, src2->get_vreg()->get_reg_id());
                    }
                    else {
                        riscv_instr = new Riscv_Instr(RISCV_ADDI, rd, rs1, 0, src2->get_basic_value().get_i32_val());
                    }
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_EQ:
                    riscv_instr = new Riscv_Instr(RISCV_SUB, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    riscv_instr = new Riscv_Instr(RISCV_SEQZ, rd, rd, 0, 0);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_NE:
                    riscv_instr = new Riscv_Instr(RISCV_SUB, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    riscv_instr = new Riscv_Instr(RISCV_SNEZ, rd, rd, 0, 0);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_GE:
                    riscv_instr = new Riscv_Instr(RISCV_SLT, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    riscv_instr = new Riscv_Instr(RISCV_XORI, rd, rd, 0, 1);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_GT:
                    riscv_instr = new Riscv_Instr(RISCV_SUB, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    riscv_instr = new Riscv_Instr(RISCV_SGTZ, rd, rd);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_LE:
                    riscv_instr = new Riscv_Instr(RISCV_SUB, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    riscv_instr = new Riscv_Instr(RISCV_SGTZ, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    riscv_instr = new Riscv_Instr(RISCV_XORI, rd, rd, 0, 1);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_LT:
                    riscv_instr = new Riscv_Instr(RISCV_SLT, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_SUB:
                    riscv_instr = new Riscv_Instr(RISCV_SUB, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_DIV:
                    riscv_instr = new Riscv_Instr(RISCV_DIV, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_MUL:
                    riscv_instr = new Riscv_Instr(RISCV_MUL, rd, rs1, src2->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_AND:
                case IR_MOD:
                case IR_OR:
                    assert(0);
                    break;
                }
                break;
            }
            case IR_BR: {
                if (instr->br_instr.get_cond()) {
                    REG rs = instr->br_instr.get_cond()->get_vreg()->get_reg_id();
                    Basic_Block_Node *true_block = instr->br_instr.get_true_edge()->get_to();
                    riscv_instr = new Riscv_Instr(RISCV_BNEZ, 0, rs, 0, 0, nullptr, block_map[true_block]);
                    new Riscv_Block_Edge(riscv_node, block_map[true_block]);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    Basic_Block_Node *false_block = instr->br_instr.get_false_edge()->get_to();
                    riscv_instr = new Riscv_Instr(RISCV_J, 0, 0, 0, 0, nullptr, block_map[false_block]);
                    new Riscv_Block_Edge(riscv_node, block_map[false_block]);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                }
                else {
                    Basic_Block_Node *true_block = instr->br_instr.get_true_edge()->get_to();
                    riscv_instr = new Riscv_Instr(RISCV_J, 0, 0, 0, 0, nullptr, block_map[true_block]);
                    new Riscv_Block_Edge(riscv_node, block_map[true_block]);
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                }
                break;
            }
            case IR_CALL: {
                riscv_instr = new Riscv_Instr(RISCV_CALL, 0, 0, 0, 0, instr->call_instr.get_func());
                riscv_node->get_info().add_instr_tail(riscv_instr);
                break;
            }
            case IR_GEP:
                assert(0);
                break;
            case IR_LOAD: {
                REG rd = instr->load_instr.get_dest()->get_reg_id();
                REG rs = instr->load_instr.get_addr()->get_vreg()->get_reg_id();
                riscv_instr = new Riscv_Instr(RISCV_LW, rd, rs, 0, 0);
                riscv_node->get_info().add_instr_tail(riscv_instr);
                break;
            }
            case IR_RETURN:
                riscv_instr = new Riscv_Instr(RISCV_JR, backend->ra);
                riscv_node->get_info().add_instr_tail(riscv_instr);
                break;
            case IR_STORE:
                riscv_instr = new Riscv_Instr(RISCV_SW, instr->store_instr.get_src()->get_vreg()->get_reg_id(), instr->store_instr.get_addr()->get_vreg()->get_reg_id());
                riscv_node->get_info().add_instr_tail(riscv_instr);
                break;
            case IR_UNARY: {
                REG rd = instr->unary_instr.get_dest()->get_reg_id();
                switch (instr->unary_instr.get_unary_opcode()) {
                case IR_ASSIGN:
                    if (instr->unary_instr.get_src()->get_op_kind() == reg) {
                        riscv_instr = new Riscv_Instr(RISCV_MOV, rd, instr->unary_instr.get_src()->get_vreg()->get_reg_id());
                        riscv_node->get_info().add_instr_tail(riscv_instr);
                        break;
                    }
                    if (instr->unary_instr.get_src()->get_op_kind() == imme) {
                        riscv_instr = new Riscv_Instr(RISCV_LI, rd, 0, 0, instr->unary_instr.get_src()->get_basic_value().get_i32_val());
                        riscv_node->get_info().add_instr_tail(riscv_instr);
                        break;
                    }
                    if (instr->unary_instr.get_src()->get_op_kind() == global_imme) {
                        riscv_instr = new Riscv_Instr(RISCV_LA, rd, 0, 0, 0, nullptr, nullptr, instr->unary_instr.get_src()->get_sym());
                        riscv_node->get_info().add_instr_tail(riscv_instr);
                        break;
                    }
                case IR_NEG:
                    riscv_instr = new Riscv_Instr(RISCV_NEG, rd, instr->unary_instr.get_src()->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                case IR_NOT:
                    riscv_instr = new Riscv_Instr(RISCV_SEQZ, rd, instr->unary_instr.get_src()->get_vreg()->get_reg_id());
                    riscv_node->get_info().add_instr_tail(riscv_instr);
                    break;
                }
                break;
            }
            }
        }
    }
}
void lowir2riscv_program(Program *prog) {
    assert(prog->get_lang_level() == LOWIR);
    for (auto &func : prog->get_funcs()) {
        lowir2riscv_func(func, prog->get_backend());
    }
    prog->set_lang_level(ASM);
}