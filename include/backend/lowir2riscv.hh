#ifndef __BACKEND_LOWIR2RISCV__
#define __BACKEND_LOWIR2RISCV__
#include <backend/backend.hh>
#include <program.hh>
void lowir2riscv_program(Program *prog);
void lowir2riscv_func(Func *func, BackEnd *backend);
#endif