#include <cstddef>
#include <func.hh>
#include <iostream>
#include <ir.hh>
#include <symbol.hh>
#include <type.hh>
#include <utils.hh>
Basic_Block_Node_Info::Basic_Block_Node_Info() {
}
void Basic_Block_Node_Info::add_instr_tail(IR_Instr *instr) {
    this->instr_list.push_back(instr);
    instr->set_block(this->block_node);
}
void Basic_Block_Node_Info::add_instr_head(IR_Instr *instr) {
    this->instr_list.push_front(instr);
    instr->set_block(this->block_node);
}
std::list<IR_Instr *> &Basic_Block_Node_Info::get_instr_list() {
    return instr_list;
}
void Basic_Block_Node_Info::set_instr_list(std::list<IR_Instr *> instr_list) {
    this->instr_list = instr_list;
}
void Basic_Block_Node_Info::add_instr_prev(IR_Instr *instr, IR_Instr *next_instr) {
    auto it = std::find(instr_list.begin(), instr_list.end(), next_instr);
    instr_list.insert(it, instr);
    instr->set_block(this->block_node);
}
void Basic_Block_Node_Info::add_instr_next(IR_Instr *instr, IR_Instr *prev_instr) {
    auto it = std::find(instr_list.begin(), instr_list.end(), prev_instr);
    instr_list.insert(++it, instr);
    instr->set_block(this->block_node);
}
void Basic_Block_Node_Info::add_instr_prev(IR_Instr *instr, std::list<IR_Instr *>::iterator next_instr) {
    instr_list.insert(next_instr, instr);
    instr->set_block(this->block_node);
}
void Basic_Block_Node_Info::add_instr_next(IR_Instr *instr, std::list<IR_Instr *>::iterator prev_instr) {
    instr_list.insert(++prev_instr, instr);
    instr->set_block(this->block_node);
}
void Basic_Block_Node_Info::delete_instr(IR_Instr *instr) {
    auto it = find(instr_list.begin(), instr_list.end(), instr);
    instr_list.erase(it);
    assert(instr->get_block() == this->block_node);
    delete instr;
}
std::list<IR_Instr *>::iterator Basic_Block_Node_Info::delete_instr(std::list<IR_Instr *>::iterator it) {
    assert((*it)->get_block() == this->block_node);
    delete *it;
    return instr_list.erase(it);
}

// it is important
Basic_Block_Node *Basic_Block_Node_Info::get_block_node() {
    return this->block_node;
}
void Basic_Block_Node_Info::set_node(Basic_Block_Node *node) {
    this->block_node = node;
}
void Basic_Block_Node_Info::add_phi(IR_VReg *phi) {
    this->bb_phi_list.push_back(phi);
    phi->set_bb_phi_def(this->block_node);
}
std::vector<IR_VReg *> &Basic_Block_Node_Info::get_phi_list() {
    return this->bb_phi_list;
}
void Basic_Block_Node_Info::print() {
    LANG_LEVEL level = this->func->get_program()->get_lang_level();
    std::cout << "b" << this->block_node->get_id();
    std::cout << "(";
    size_t i = 0;
    for (auto &phi : this->bb_phi_list) {
        if (i) std::cout << ",";
        i++;
        phi->print(level);
    }
    std::cout << ")";

    std::cout << ": ";
    std::cout << "                  preds:";
    i = 0;
    for (auto pred : this->block_node->get_preds()) {
        if (i != 0) {
            std::cout << ", ";
        }
        std::cout << "b" << pred->get_from()->get_id();
        i++;
    }
    std::cout << std::endl;

    for (auto instr : instr_list) {
        instr->print(level);
    }
}
void Basic_Block_Node_Info::set_func(Func *func) {
    this->func = func;
}
Func *Basic_Block_Node_Info::get_func() {
    return this->func;
}
void Basic_Block_Node_Info::move_instr_list(std::list<IR_Instr *>::iterator insert_loc, std::list<IR_Instr *> list, std::list<IR_Instr *>::iterator start, std::list<IR_Instr *>::iterator end) {
    this->instr_list.insert(insert_loc, start, end);
    auto it_next = start;
    while (it_next != end) {
        auto it = it_next++;
        (*it)->set_block(this->block_node);
        list.erase(it);
    }
}
Basic_Block_Node_Info::~Basic_Block_Node_Info() {
    for (auto &phi : this->bb_phi_list)
        assert(phi->get_def_bb() == this->block_node);
    for (auto instr : this->instr_list) {
        assert(instr->get_block() == this->block_node);
        delete instr;
    }
}

void Basic_Block_Edge_Info::set_edge(Basic_Block_Edge *edge) {
    this->bb_edge = edge;
}
void Basic_Block_Edge_Info::add_bb_param(IR_Operation *op) {
    bb_param_list.push_back(op);
    op->set_bb_param_use(this->bb_edge);
}
void Basic_Block_Edge_Info::print_bb_params(LANG_LEVEL level) {
    size_t i = 0;
    std::cout << "(";
    for (auto &param : this->bb_param_list) {
        if (i != 0) std::cout << ",";
        i++;
        param->print(level);
    }
    std::cout << ")";
}
Basic_Block_Edge_Info::~Basic_Block_Edge_Info() {
    for (auto &op : this->bb_param_list) {
        assert(op->get_use_bb_edge() == this->bb_edge);
        delete op;
    }
}
IR_VReg::IR_VReg(Type *type, size_t id) {
    this->p_type = type;
    this->vreg_id = id;
    reg_id = -1;
}
IR_VReg::IR_VReg(Type *type, size_t id, IR_Instr *instructon) {
    this->p_type = type;
    this->vreg_id = id;
    this->def_type = instr_def;
    this->instr = instructon;
    reg_id = -1;
}
IR_VReg::IR_VReg(Type *type, size_t id, Func *func, size_t param_id) {
    this->p_type = type;
    this->vreg_id = id;
    this->param_id = param_id;
    this->func = func;
    this->def_type = param_def;
    reg_id = -1;
}
IR_VReg *IR_VReg::copy() {
    switch (this->def_type) {
    case instr_def:
        return new IR_VReg(this->p_type->copy(), this->vreg_id, this->instr);
    case param_def:
        return new IR_VReg(this->p_type->copy(), this->vreg_id, this->func, this->param_id);
    case bb_phi_def:
        return new IR_VReg(this->p_type->copy(), this->vreg_id);
    }
    assert(0);
}
void IR_VReg::add_use(IR_Operation *use) {
    this->use_list.push_back(use);
}
void IR_VReg::set_id(size_t id) {
    this->vreg_id = id;
}
void IR_VReg::set_instr_def(IR_Instr *instr) {
    this->def_type = instr_def;
    this->instr = instr;
}
void IR_VReg::set_bb_phi_def(Basic_Block_Node *bb) {
    this->def_type = bb_phi_def;
    this->bb = bb;
}
void IR_VReg::set_param_def(Func *func, size_t param_id) {
    this->def_type = param_def;
    this->func = func;
    this->param_id = param_id;
}
std::vector<IR_Operation *> IR_VReg::get_use_list() {
    return this->use_list;
}
bool IR_VReg::delete_use(IR_Operation *op) {
    auto it = find(this->use_list.begin(), this->use_list.end(), op);
    if (it == this->use_list.end())
        return false;
    this->use_list.erase(it);
    return true;
}
Type *IR_VReg::get_type() {
    return this->p_type;
}
Basic_Block_Node *IR_VReg::get_def_bb() {
    assert(this->def_type == bb_phi_def);
    return this->bb;
}
IR_Instr *IR_VReg::get_def_instr() {
    assert(this->def_type == instr_def);
    return this->instr;
}
void IR_VReg::print(LANG_LEVEL level) {
    if (level == LOWIR) {
        std::cout << " REG:" << reg_id;
        return;
    }
    p_type->print();
    std::cout << " %" << vreg_id << "";
}
Def_Type IR_VReg::get_def_type() {
    return this->def_type;
}
IR_VReg::~IR_VReg() {
    assert(this->use_list.size() == 0);
    delete this->p_type;
}
IR_Operation::IR_Operation(Symbol *p_global_sym) {
    this->p_global_sym = p_global_sym;
    this->op_kind = global_imme;
}
IR_Operation::IR_Operation(IR_VReg *p_vreg) {
    this->p_vreg = p_vreg;
    this->op_kind = reg;
    p_vreg->add_use(this);
}
IR_Operation::IR_Operation(Basic_Value val) {
    this->val = val;
    this->op_kind = imme;
}
void IR_Operation::reset_vreg(IR_VReg *p_reg) {
    if (this->op_kind == reg)
        this->p_vreg->delete_use(this);
    this->op_kind = reg;
    this->p_vreg = p_reg;
    p_reg->add_use(this);
}
void IR_Operation::set_instr_use(IR_Instr *instr) {
    this->used_type = instr_use;
    this->p_instr = instr;
}
void IR_Operation::set_bb_param_use(Basic_Block_Edge *bb_edge) {
    this->used_type = bb_param_use;
    this->bb_edge = bb_edge;
}
Type *IR_Operation::get_type() {
    switch (this->op_kind) {
    case reg:
        return this->p_vreg->get_type();
    case imme:
        return this->val.get_type();
    case global_imme:
        return this->p_global_sym->get_type();
    }
    return nullptr;
}
IR_Operation *IR_Operation::copy() {
    switch (this->op_kind) {
    case reg:
        return new IR_Operation(this->p_vreg);
    case imme:
        return new IR_Operation(this->val);
    case global_imme:
        return new IR_Operation(this->p_global_sym);
    }
    assert(0);
}
IR_Instr *IR_Operation::get_use_instr() {
    assert(this->used_type == instr_use);
    return this->p_instr;
}
Basic_Block_Edge *IR_Operation::get_use_bb_edge() {
    assert(this->used_type == bb_param_use);
    return this->bb_edge;
}

void IR_Operation::print(LANG_LEVEL level) {
    switch (op_kind) {
    case reg:
        p_vreg->print(level);
        break;
    case imme:
        std::cout << val;
        break;
    case global_imme:
        p_global_sym->print(level);
        break;
    }
}
IR_Operation::~IR_Operation() {
    if (this->op_kind == reg) {
        this->p_vreg->delete_use(this);
    }
}
IR_Binary_Instr::IR_Binary_Instr(IR_Binary_Opcode opcode, IR_VReg *dest, IR_Operation *src1, IR_Operation *src2) {
    this->opcode = opcode;
    this->dest = dest;
    this->src1 = src1;
    this->src2 = src2;
}
IR_Binary_Opcode IR_Binary_Instr::get_opcode() {
    return opcode;
}
void IR_Binary_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    dest->print(level);
    std::cout << " =";
    switch (opcode) {
    case IR_ADD:
        std::cout << " add";
        break;
    case IR_SUB:
        std::cout << " sub";
        break;
    case IR_MUL:
        std::cout << " mul";
        break;
    case IR_DIV:
        std::cout << " div";
        break;
    case IR_MOD:
        std::cout << " mod";
        break;
    case IR_AND:
        std::cout << " and";
        break;
    case IR_OR:
        std::cout << " or";
        break;
    // case IR_XOR:
    //     std::cout << "xor ";
    //     break;
    // case IR_SHL:
    //     std::cout << "shl ";
    //     break;
    // case IR_SHR:
    //     std::cout << "shr ";
    //     break;
    case IR_EQ:
        std::cout << " eq";
        break;
    case IR_NE:
        std::cout << " ne";
        break;
    case IR_LT:
        std::cout << " lt";
        break;
    case IR_LE:
        std::cout << " le";
        break;
    case IR_GT:
        std::cout << " gt";
        break;
    case IR_GE:
        std::cout << " ge";
        break;
    }
    src1->print(level);
    std::cout << ",";
    src2->print(level);
    std::cout << std::endl;
}
IR_Unary_Instr::IR_Unary_Instr(IR_Unary_Opcode opcode, IR_VReg *dest, IR_Operation *src) {
    this->opcode = opcode;
    this->dest = dest;
    this->src = src;
}
void IR_Unary_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    dest->print(level);
    std::cout << " = ";
    switch (opcode) {
    case IR_NEG:
        std::cout << " neg";
        break;
    case IR_NOT:
        std::cout << " not";
        break;
    // case IR_INC:
    //     std::cout << " inc";
    //     break;
    // case IR_DEC:
    //     std::cout << " dec";
    //     break;
    case IR_ASSIGN:
        break;
    }
    src->print(level);
    std::cout << std::endl;
}
IR_Store_Instr::IR_Store_Instr(IR_Operation *src, IR_Operation *addr) {
    this->src = src;
    this->addr = addr;
}
void IR_Store_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    std::cout << " store";
    src->print(level);
    std::cout << ",";
    addr->print(level);
    std::cout << std::endl;
}

IR_Load_Instr::IR_Load_Instr(IR_VReg *dest, IR_Operation *addr) {
    this->dest = dest;
    this->addr = addr;
}
void IR_Load_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    dest->print(level);
    std::cout << " = load";
    addr->print(level);
    std::cout << std::endl;
}
IR_Call_Instr::IR_Call_Instr(Func *func, IR_VReg *dest) {
    this->func = func;
    this->dest = dest;
}
void IR_Call_Instr::add_arg(IR_Operation *arg) {
    this->args.push_back(arg);
}
std::vector<IR_Operation *> &IR_Call_Instr::get_args() {
    return this->args;
}
void IR_Call_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    if (dest) {
        dest->print(level);
        std::cout << " =";
    }
    else
        assert(func->get_is_void());
    std::cout << " call " << func->get_name();
    std::cout << "(";
    size_t i = 0;
    for (auto arg : args) {
        if (i != 0) std::cout << ", ";
        i++;
        arg->print(level);
    }
    std::cout << ")" << std::endl;
}

IR_Gep_Instr::IR_Gep_Instr(IR_VReg *dest, IR_Operation *src, IR_Operation *idx1, IR_Operation *idx2) {
    this->dest = dest;
    this->src = src;
    this->idx1 = idx1;
    this->idx2 = idx2;
    this->if_idx2 = true;
}
IR_Gep_Instr::IR_Gep_Instr(IR_VReg *dest, IR_Operation *src, IR_Operation *idx1) {
    this->dest = dest;
    this->src = src;
    this->idx1 = idx1;
    this->if_idx2 = false;
}
void IR_Gep_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    dest->print(level);
    std::cout << " = gep";
    src->print(level);
    std::cout << ",";
    idx1->print(level);
    if (this->if_idx2) {
        std::cout << ",";
        idx2->print(level);
    }
    std::cout << std::endl;
}
IR_Alloca_Instr::IR_Alloca_Instr(IR_VReg *des, Symbol *p_stack_sym) {
    this->p_stack_sym = p_stack_sym;
    this->dest = des;
}

void IR_Alloca_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    dest->print(level);
    std::cout << " = alloca";
    p_stack_sym->print(level);
    std::cout << std::endl;
}
IR_Br_Instr::IR_Br_Instr(IR_Operation *cond, Basic_Block_Edge *true_bb, Basic_Block_Edge *false_bb) {
    this->cond = cond;
    this->true_bb = true_bb;
    this->false_bb = false_bb;
}
IR_Br_Instr::IR_Br_Instr(Basic_Block_Edge *true_bb) {
    this->true_bb = true_bb;
    this->cond = nullptr;
    this->false_bb = nullptr;
}
void IR_Br_Instr::set_true_edge(Basic_Block_Edge *true_bb) {
    this->true_bb = true_bb;
}
void IR_Br_Instr::set_false_edge(Basic_Block_Edge *false_bb) {
    this->false_bb = false_bb;
}
void IR_Br_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    std::cout << " br";
    if (cond) {
        cond->print(level);
        std::cout << ", ";
    }
    std::cout << " b" << true_bb->get_to()->get_id();
    true_bb->get_info().print_bb_params(level);
    if (false_bb) {
        std::cout << ", b" << false_bb->get_to()->get_id();
        false_bb->get_info().print_bb_params(level);
    }
    std::cout << std::endl;
}

IR_Ret_Instr::IR_Ret_Instr(IR_Operation *ret_val) {
    this->ret_val = ret_val;
}
void IR_Ret_Instr::print(LANG_LEVEL level) {
    std::cout << "  ";
    std::cout << " ret";
    if (ret_val) ret_val->print(level);
    std::cout << std::endl;
}
IR_Instr_Type IR_Instr::get_instr_type() {
    return this->instr_type;
}
IR_Instr::IR_Instr(IR_Binary_Instr binary_instr) {
    this->instr_type = IR_BINARY;
    this->binary_instr = binary_instr;
    this->binary_instr.dest->set_instr_def(this);
    this->binary_instr.src1->set_instr_use(this);
    this->binary_instr.src2->set_instr_use(this);
}
IR_Instr::IR_Instr(IR_Unary_Instr unary_instr) {
    this->instr_type = IR_UNARY;
    this->unary_instr = unary_instr;
    this->unary_instr.dest->set_instr_def(this);
    this->unary_instr.src->set_instr_use(this);
}
IR_Instr::IR_Instr(IR_Store_Instr store_instr) {
    this->instr_type = IR_STORE;
    this->store_instr = store_instr;
    this->store_instr.addr->set_instr_use(this);
    this->store_instr.src->set_instr_use(this);
}
IR_Instr::IR_Instr(IR_Load_Instr load_instr) {
    this->instr_type = IR_LOAD;
    this->load_instr = load_instr;
    this->load_instr.dest->set_instr_def(this);
    this->load_instr.addr->set_instr_use(this);
}
IR_Instr::IR_Instr(IR_Call_Instr call_instr) {
    this->instr_type = IR_CALL;
    this->call_instr = call_instr;
    if (this->call_instr.dest)
        this->call_instr.dest->set_instr_def(this);
    for (auto arg : this->call_instr.args)
        arg->set_instr_use(this);
}
IR_Instr::IR_Instr(IR_Gep_Instr gep_instr) {
    this->instr_type = IR_GEP;
    this->gep_instr = gep_instr;
    this->gep_instr.dest->set_instr_def(this);
    this->gep_instr.src->set_instr_use(this);
    this->gep_instr.idx1->set_instr_use(this);
    if (this->gep_instr.if_idx2)
        this->gep_instr.idx2->set_instr_use(this);
}
IR_Instr::IR_Instr(IR_Alloca_Instr alloca_instr) {
    this->instr_type = IR_ALLOCA;
    this->alloca_instr = alloca_instr;
    this->alloca_instr.dest->set_instr_def(this);
}
IR_Instr::IR_Instr(IR_Br_Instr br_instr) {
    this->instr_type = IR_BR;
    this->br_instr = br_instr;
    if (this->br_instr.cond)
        this->br_instr.cond->set_instr_use(this);
}
IR_Instr::IR_Instr(IR_Ret_Instr ret_instr) {
    this->instr_type = IR_RETURN;
    this->ret_instr = ret_instr;
    if (this->ret_instr.ret_val)
        this->ret_instr.ret_val->set_instr_use(this);
}
void IR_Instr::set_block(Basic_Block_Node *bb) {
    this->basic_block = bb;
}
Basic_Block_Node *IR_Instr::get_block() {
    return this->basic_block;
}
void IR_Instr::call_add_arg(IR_Operation *arg) {
    assert(this->instr_type == IR_CALL);
    this->call_instr.add_arg(arg);
    arg->set_instr_use(this);
}
IR_VReg *IR_Instr::get_des() {
    switch (this->instr_type) {
    case IR_ALLOCA:
        return this->alloca_instr.dest;
    case IR_BINARY:
        return this->binary_instr.dest;
    case IR_CALL:
        return this->call_instr.dest;
    case IR_GEP:
        return this->gep_instr.dest;
    case IR_LOAD:
        return this->load_instr.dest;
    case IR_UNARY:
        return this->unary_instr.dest;
    case IR_STORE:
    case IR_BR:
    case IR_RETURN:
        return nullptr;
    }
    return nullptr;
}
void IR_Instr::print(LANG_LEVEL level) {
    switch (this->instr_type) {
    case IR_BINARY:
        binary_instr.print(level);
        break;
    case IR_UNARY:
        unary_instr.print(level);
        break;
    case IR_STORE:
        store_instr.print(level);
        break;
    case IR_LOAD:
        load_instr.print(level);
        break;
    case IR_CALL:
        call_instr.print(level);
        break;
    case IR_GEP:
        gep_instr.print(level);
        break;
    case IR_ALLOCA:
        alloca_instr.print(level);
        break;
    case IR_BR:
        br_instr.print(level);
        break;
    case IR_RETURN:
        ret_instr.print(level);
        break;
    }
}

IR_Instr::~IR_Instr() {
    switch (this->instr_type) {
    case IR_BINARY:
        assert(this->binary_instr.dest->get_def_instr() == this);
        assert(this->binary_instr.src1->get_use_instr() == this);
        assert(this->binary_instr.src2->get_use_instr() == this);
        delete this->binary_instr.src1;
        delete this->binary_instr.src2;
        this->binary_instr.dest->set_instr_def(nullptr);
        break;
    case IR_UNARY:
        assert(this->unary_instr.dest->get_def_instr() == this);
        assert(this->unary_instr.src->get_use_instr() == this);
        delete this->unary_instr.src;
        this->unary_instr.dest->set_instr_def(nullptr);
        break;
    case IR_ALLOCA:
        assert(this->alloca_instr.dest->get_def_instr() == this);
        this->alloca_instr.dest->set_instr_def(nullptr);
        break;
    case IR_GEP:
        assert(this->gep_instr.dest->get_def_instr() == this);
        assert(this->gep_instr.src->get_use_instr() == this);
        assert(this->gep_instr.idx1->get_use_instr() == this);
        if (this->gep_instr.if_idx2)
            assert(this->gep_instr.idx2->get_use_instr() == this);
        delete this->gep_instr.src;
        delete this->gep_instr.idx1;
        if (this->gep_instr.if_idx2)
            delete this->gep_instr.idx2;
        this->gep_instr.dest->set_instr_def(nullptr);
        break;
    case IR_LOAD:
        assert(this->load_instr.dest->get_def_instr() == this);
        assert(this->load_instr.addr->get_use_instr() == this);
        delete this->load_instr.addr;
        this->load_instr.dest->set_instr_def(nullptr);
        break;
    case IR_CALL:
        if (this->call_instr.dest) {
            assert(this->call_instr.dest->get_def_instr() == this);
            this->call_instr.dest->set_instr_def(nullptr);
        }
        for (auto param : this->call_instr.args) {
            assert(param->get_use_instr() == this);
            delete param;
        }
        break;
    case IR_RETURN:
        if (!this->ret_instr.ret_val) return;
        assert(this->ret_instr.ret_val->get_use_instr() == this);
        delete this->ret_instr.ret_val;
        break;
    case IR_BR:
        if (this->br_instr.cond) {
            assert(this->br_instr.cond->get_use_instr() == this);
            delete this->br_instr.cond;
        }
        break;
    case IR_STORE:
        assert(this->store_instr.src->get_use_instr() == this);
        assert(this->store_instr.addr->get_use_instr() == this);
        delete this->store_instr.addr;
        delete this->store_instr.src;
        break;
        // do not delete edge
        break;
    }
}
