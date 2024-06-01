#include <copy_propagation.hh>
#include <symbol.hh>
void copy_propagation_func(Func *func) {
}
void copy_propagation_program(Program *program) {
    for (auto &func : program->get_funcs()) {
        copy_propagation_func(func);
    }
}
