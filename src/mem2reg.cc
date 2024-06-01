#include <bitmap.hh>
#include <cstddef>
#include <ir.hh>
#include <map>
#include <mem2reg.hh>
#include <symbol.hh>
#include <type.hh>
#include <utils.hh>
#include <vector>

static void rename(Func *func, Basic_Block_Node *bb, std::vector<BitMap> &phis_set, std::map<size_t, IR_VReg *> id2reg, std::map<IR_VReg *, IR_VReg *> current_sym) {
    if (bb->is_visited) return;
    bb->is_visited = true;
    // 处理phi
    BitMap &map = phis_set[bb->get_id()];
    for (size_t i = 0; i < map.get_element_num(); i++) {
        if (map.if_in(i)) {
            IR_VReg *sym_addr = id2reg[i];
            Type *type = sym_addr->get_type()->copy();
            type->ptr_level_dec();
            IR_VReg *new_vreg = new IR_VReg(type);
            func->add_local_vreg(new_vreg);
            current_sym[sym_addr] = new_vreg;
            bb->get_info().add_phi(new_vreg);
        }
    }
    auto it_next = bb->get_info().get_instr_list().begin();
    while (it_next != bb->get_info().get_instr_list().end()) {
        auto it = it_next++;
        IR_Instr *instr = *it;
        if (instr->get_instr_type() == IR_LOAD) {
            if (instr->load_instr.get_addr()->get_op_kind() == reg) {
                IR_VReg *addr = instr->load_instr.get_addr()->get_vreg();
                IR_VReg *dest = instr->load_instr.get_dest();
                auto sym = current_sym.find(addr);
                if (sym == current_sym.end()) continue;
                assert(sym->second);
                bb->get_info().delete_instr(it);
                IR_Instr *new_assign = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, dest, new IR_Operation(sym->second)));
                bb->get_info().add_instr_prev(new_assign, it_next);
                continue;
            }
        }
        if (instr->get_instr_type() == IR_STORE) {
            if (instr->store_instr.get_addr()->get_op_kind() == reg) {
                IR_VReg *addr = instr->store_instr.get_addr()->get_vreg();
                IR_Operation *src = instr->store_instr.get_src();
                auto sym = current_sym.find(addr);
                if (sym == current_sym.end()) continue;
                IR_VReg *new_sym = new IR_VReg(src->get_type()->copy());
                current_sym[addr] = new_sym;
                func->add_local_vreg(new_sym);
                IR_Instr *new_assign = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, new_sym, src->copy()));
                bb->get_info().add_instr_next(new_assign, it);
                bb->get_info().delete_instr(it);
                continue;
            }
        }
    }
    map.print();
    // 处理基本块参数
    for (auto &edge : bb->get_succs()) {
        Basic_Block_Node *to = edge->get_to();
        BitMap &map = phis_set[to->get_id()];
        for (size_t i = 0; i < map.get_element_num(); i++) {
            if (map.if_in(i)) {
                if (current_sym[id2reg[i]])
                    edge->get_info().add_bb_param(new IR_Operation(current_sym[id2reg[i]]));
                else
                    edge->get_info().add_bb_param(nullptr);
            }
        }
        rename(func, to, phis_set, id2reg, current_sym);
    }
}
static void insert_phi(Func *func) {
    Basic_Block_Graph *graph = func->get_block_graph();
    size_t block_num = graph->get_nodes().size();
    size_t vreg_num = func->get_vreg_num();
    // func->set_seqence_vreg_id();
    // 得到每个基本块的定义变量
    std::vector<BitMap> defs_set(block_num, BitMap(vreg_num));
    std::map<size_t, IR_VReg *> id2reg; // 从 reg 编号到 vreg 的映射
    for (auto &map : defs_set)
        map.set_empty();
    for (auto &reg : func->get_local_reg_list()) {
        if (reg->get_def_type() == instr_def) {
            if (reg->get_def_instr()->get_instr_type() == IR_ALLOCA) {
                for (auto &op : reg->get_use_list()) {
                    if (op->get_use_type() == instr_use) {
                        id2reg[reg->get_id()] = reg;
                        IR_Instr *use_instr = op->get_use_instr();
                        if (use_instr->get_instr_type() == IR_STORE)
                            defs_set[use_instr->get_block()->get_id()].add(reg->get_id());
                    }
                }
            }
        }
    }

    for (auto &map : defs_set)
        map.print();
    // for (auto &node : graph->get_nodes()) {
    //     for (auto &instr : node->get_info().get_instr_list()) {
    //         if (instr->get_instr_type() == IR_STORE) {
    //             IR_Operation *addr = instr->store_instr.get_addr();
    //             if (addr->get_op_kind() == reg) {
    //                 IR_VReg *vreg = addr->get_vreg();
    //                 if (vreg->get_def_instr()->get_instr_type() == IR_GEP) continue;
    //                 map_defs[node->get_id()].add(vreg->get_id());
    //             }
    //         }
    //     }
    // }

    // 迭代得到每个基本块需插入的 phi
    std::vector<BitMap> phis_set(block_num, BitMap(vreg_num));
    for (auto &map : phis_set)
        map.set_empty();
    for (auto &node : graph->get_nodes()) {
        for (auto &dom_frontier : node->get_dom_frontier()) {
            // defs_set[node->get_id()].print();
            phis_set[dom_frontier->get_id()] = phis_set[dom_frontier->get_id()] | defs_set[node->get_id()];
        }
    }
    std::queue<Basic_Block_Node *> worklist;
    for (auto node : graph->get_nodes())
        worklist.push(node);
    while (!worklist.empty()) {
        Basic_Block_Node *node = worklist.front();
        worklist.pop();
        for (auto dom_frontier : node->get_dom_frontier()) {
            BitMap tmp = phis_set[dom_frontier->get_id()] | phis_set[node->get_id()];
            if (tmp != phis_set[dom_frontier->get_id()]) {
                worklist.push(dom_frontier);
                phis_set[dom_frontier->get_id()] = tmp;
            }
        }
    }

    // 以上得到每个基本块的 phi 包含的地址，接下来重命名并插入 phi
    std::map<IR_VReg *, IR_VReg *> current_sym;
    for (auto &t : id2reg)
        current_sym[t.second] = nullptr;
    graph->init_visited();
    rename(func, func->get_entry_bb(), phis_set, id2reg, current_sym);
}

void mem2reg_func(Func *func) {
    if (func->get_is_lib()) return;
    func->get_block_graph()->compute_dom(func->get_entry_bb());
    func->get_block_graph()->print_idom_tree(func->get_entry_bb());
    func->get_block_graph()->compute_dom_frontier(func->get_entry_bb());
    func->get_block_graph()->print_dom_frontiers();
    insert_phi(func);
    func->reset_vreg_id();
}

void mem2reg_program(Program *program) {
    assert(program->get_lang_level() == IR);
    for (auto func : program->get_funcs()) {
        mem2reg_func(func);
    }
}