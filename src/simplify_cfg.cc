#include <simplify_cfg.hh>
static void merge_single_pred_block(Func *func) {
    auto it = func->get_block_graph()->get_nodes().begin();
    while (it != func->get_block_graph()->get_nodes().end()) {
        Basic_Block_Node *node = *it;
        if (node->get_succs().size() == 1 && node->get_preds().size() == 1) {
            Basic_Block_Node *from = node->get_preds()[0]->get_from();
            auto insert_loc = prev(from->get_info().get_instr_list().end());
            auto start = node->get_info().get_instr_list().begin();
            auto end = prev(node->get_info().get_instr_list().end());
            from->get_info().move_instr_list(insert_loc, node->get_info().get_instr_list(), start, end);
            assert((*insert_loc)->get_instr_type() == IR_BR);
            if ((*insert_loc)->br_instr.get_true_edge() == node->get_preds()[0])
                (*insert_loc)->br_instr.set_true_edge(node->get_succs()[0]);
            if ((*insert_loc)->br_instr.get_false_edge() == node->get_preds()[0])
                (*insert_loc)->br_instr.set_false_edge(node->get_succs()[0]);
            node->get_succs()[0]->reset_from(from);
            delete node;
            it = func->get_block_graph()->get_nodes().erase(it);
            continue;
        }
        it++;
    }
}
void simplifycfg_func(Func *func) {
    if (func->get_is_lib()) return;
    func->get_block_graph()->get_connected_graph(func->get_entry_bb());
    if (0)
        merge_single_pred_block(func); // 合并单一前驱
    func->clear_unused_vregs();
    func->reset_block_id();
    func->reset_vreg_id();
}
void simplifycfg_program(Program *prog) {
    assert(prog->get_lang_level() == IR || prog->get_lang_level() == LOWIR);
    for (auto &func : prog->get_funcs())
        simplifycfg_func(func);
}