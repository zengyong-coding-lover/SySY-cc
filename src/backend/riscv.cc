#include <backend/riscv.hh>
#include <func.hh>

Riscv_Instr::Riscv_Instr(Riscv_Instr_Type type, REG rd, REG rs1, REG rs2, int imme12, Func *func, Riscv_Block_Node *target, Symbol *global_sym) {
    this->type = type;
    this->rs1 = rs1;
    this->rs2 = rs2;
    this->rd = rd;
    this->imme12 = imme12;
    this->func = func;
    this->target = target;
    this->global_sym = global_sym;
}
std::ostream &operator<<(std::ostream &os, const Riscv_Instr &instr) {
    os << "   ";
    switch (instr.type) {
    case RISCV_ADDI:
        os << "addi ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << instr.imme12;
        break;
    case RISCV_BEQ:
        os << "beq ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        os << ", ";
        os << instr.target->get_info().get_name();
        break;
    case RISCV_BGE:
        os << "bge ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        os << ", ";
        os << instr.target->get_info().get_name();
        break;
    case RISCV_BLE:
        os << "ble ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        os << ", ";
        os << instr.target->get_info().get_name();
        break;
    case RISCV_BNE:
        os << "bne ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        os << ", ";
        os << instr.target->get_info().get_name();
        break;
    case RISCV_BNEZ:
        os << "bnez ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << instr.target->get_info().get_name();
        break;
    case RISCV_CALL:
        os << "call ";
        os << instr.func->get_name();
        break;
    case RISCV_LW:
        os << "lw ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << instr.imme12;
        os << "(" << get_reg_name(instr.rs1) << ")";
        break;
    case RISCV_SW:
        os << "sw ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << instr.imme12;
        os << "(" << get_reg_name(instr.rs1) << ")";
        break;
    case RISCV_LI:
        os << "li ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << instr.imme12;
        break;
    case RISCV_MOV:
        os << "mv ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        break;
    case RISCV_JR:
        os << "jr ";
        os << get_reg_name(instr.rd);
        break;
    case RISCV_SUB:
        os << "sub ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        break;
    case RISCV_ADD:
        os << "add ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        break;
    case RISCV_SEQZ:
        os << "seqz ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        break;
    case RISCV_SGTZ:
        os << "sgtz ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        break;
    case RISCV_SLT:
        os << "slt ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        break;
    case RISCV_SNEZ:
        os << "snez ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        break;
    case RISCV_XORI:
        os << "xori ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << instr.imme12;
        break;
    case RISCV_J:
        os << "j ";
        os << instr.target->get_info().get_name();
        break;
    case RISCV_NEG:
        os << "neg ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        break;
    case RISCV_LA:
        os << "la ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << instr.global_sym->get_name();
        break;
    case RISCV_MUL:
        os << "mul ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
        break;
    case RISCV_DIV:
        os << "div ";
        os << get_reg_name(instr.rd);
        os << ", ";
        os << get_reg_name(instr.rs1);
        os << ", ";
        os << get_reg_name(instr.rs2);
    }
    os << std::endl;
    return os;
}

std::string Riscv_Block_Node_Info::get_name() {
    return func->get_name() + "_b" + std::to_string(this->node->get_id());
}
std::ostream &operator<<(std::ostream &os, Riscv_Block_Node_Info &info) {
    os << info.get_name();
    os << ":" << std::endl;
    for (auto &instr : info.instr_list) {
        os << *instr;
    }
    return os;
}