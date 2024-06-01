#include <cstddef>
#include <dead_code_eliminate.hh>
#include <func.hh>
#include <ir.hh>
#include <symbol.hh>
#include <utils.hh>
#include <vector>
void dead_code_eliminate_func(Func *func) {
    std::vector<bool> has_deal(func->get_vreg_num(), false);
    std::queue<IR_Operation *> work_queue;
    // 初始化工作列表
    for (auto &node : func->get_basic_blocks()) {
        for (auto &instr : node->get_info().get_instr_list()) {
            if (instr->get_instr_type() == IR_STORE) {
                work_queue.push(instr->store_instr.get_addr());
                work_queue.push(instr->store_instr.get_src());
                continue;
            }
            if (instr->get_instr_type() == IR_CALL) {
                for (auto &param : instr->call_instr.get_args())
                    work_queue.push(param);
                continue;
            }
            if (instr->get_instr_type() == IR_RETURN) {
                if (instr->ret_instr.get_ret_val())
                    work_queue.push(instr->ret_instr.get_ret_val());
                continue;
            }
            if (instr->get_instr_type() == IR_BR) {
                if (instr->br_instr.get_cond())
                    work_queue.push(instr->br_instr.get_cond());
                continue;
            }
        }
    }

    // 处理工作集
    while (!work_queue.empty()) {
        IR_Operation *op = work_queue.front();
        work_queue.pop();
        if (op->get_op_kind() != reg) continue;
        IR_VReg *vreg = op->get_vreg();
        if (has_deal[vreg->get_id()]) continue;
        has_deal[vreg->get_id()] = true;
        if (vreg->get_def_type() == instr_def) {
            IR_Instr *def_instr = vreg->get_def_instr();
            switch (def_instr->get_instr_type()) {
            case IR_LOAD:
                work_queue.push(def_instr->load_instr.get_addr());
                break;
            case IR_ALLOCA:
                break;
            case IR_BINARY:
                work_queue.push(def_instr->binary_instr.get_src1());
                work_queue.push(def_instr->binary_instr.get_src2());
                break;
            case IR_UNARY:
                work_queue.push(def_instr->unary_instr.get_src());
                break;
            case IR_GEP:
                work_queue.push(def_instr->gep_instr.get_src());
                work_queue.push(def_instr->gep_instr.get_idx1());
                if (def_instr->gep_instr.get_if_idx2())
                    work_queue.push(def_instr->gep_instr.get_idx2());
                break;
            case IR_BR:
            case IR_CALL:
            case IR_STORE:
            case IR_RETURN:
                break;
            }
            continue;
        }
        if (vreg->get_def_type() == bb_phi_def) {
            std::vector<IR_VReg *> &phi_list = vreg->get_def_bb()->get_info().get_phi_list();
            size_t index = find(phi_list.begin(), phi_list.end(), vreg) - phi_list.begin();
            for (auto &pred_edge : vreg->get_def_bb()->get_preds())
                work_queue.push(pred_edge->get_info().get_bb_param()[index]);
            continue;
        }
    }

    // 删除无用指令
    for (auto &node : func->get_basic_blocks()) {
        std::vector<IR_VReg *> &phi_list = node->get_info().get_phi_list();
        auto it1 = phi_list.begin();
        while (it1 != phi_list.end()) {
            IR_VReg *phi = *it1;
            if (has_deal[phi->get_id()]) {
                it1++;
                continue;
            }
            it1 = phi_list.erase(it1);
            phi->set_bb_phi_def(nullptr);
            size_t index = it1 - phi_list.begin();
            for (auto &pred_edge : node->get_preds()) {
                std::vector<IR_Operation *> &param_list = pred_edge->get_info().get_bb_param();
                delete param_list[index];
                param_list.erase(param_list.begin() + index);
            }
        }
        std::list<IR_Instr *> &instr_list = node->get_info().get_instr_list();
        auto it2 = instr_list.begin();
        while (it2 != instr_list.end()) {
            IR_Instr *instr = *it2;
            IR_VReg *des = instr->get_des();
            if (!des || has_deal[des->get_id()]) {
                it2++;
                continue;
            }
            delete instr;
            it2 = instr_list.erase(it2);
        }
    }

    // 删除 无用 vreg;
    func->clear_unused_vregs();
}
void dead_code_eliminate_program(Program *program) {
    assert(program->get_lang_level() == IR || program->get_lang_level() == LOWIR);
    for (auto &func : program->get_funcs()) {
        dead_code_eliminate_func(func);
    }
}