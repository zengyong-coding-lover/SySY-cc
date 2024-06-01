#ifndef __PASSMANAGER__
#define __PASSMANAGER__
#include <program.hh>
typedef enum Pass_Kind {
    AST2IR,
    SIMPLIFY_CFG,
    LOWER_IR,
    WHOLE_IN_MEM_ALLOCA,
    AFTER_ALLOCA,
    DEAD_CODE_ELIMINATE,
    MEM2REG,
    COPY_PROPAGATION,
    LOWIR_ASM,
    STANDRAD_LOWIR,
} Pass_Kind;
class PassManager {
private:
    std::queue<Pass_Kind> passes;
    Program *program;

public:
    PassManager(Program *program);
    void add_pass(Pass_Kind pass);
    void run();
};
#endif