#include <backend/lowir2riscv.hh>
#include <reg_alloca/after_alloca.hh>
#include <ast2ir.hh>
#include <lowerir.hh>
#include <passmanager.hh>
#include <program.hh>
#include <reg_alloca/whole_in_mem.hh>
#include <standrad_lowir.hh>
#include <simplify_cfg.hh>
PassManager::PassManager(Program *program) {
    this->program = program;
}
void PassManager::add_pass(Pass_Kind pass) {
    passes.push(pass);
}
void PassManager::run() {
    while (!passes.empty()) {
        Pass_Kind pass = passes.front();
        passes.pop();
        switch (pass) {
        case AST2IR:
            Ast2IR_Program(program);
            break;
        case SIMPLIFY_CFG:
            simplifycfg_program(program);
            break;
        case LOWER_IR:
            lowerir_program(program);
            break;
        case STANDRAD_LOWIR:
            standrad_lowir_program(program);
            break;
        case WHOLE_IN_MEM_ALLOCA:
            whole_in_mem_alloca_program(program);
            break;
        case AFTER_ALLOCA:
            after_alloca_program(program);
            break;
        case LOWIR_ASM:
            switch (program->get_arch()) {
            case RISCV32:
                lowir2riscv_program(program);
                break;
            case ARMv7:
                break;
                assert(0);
            }
        }
    }
}