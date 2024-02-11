#ifndef __REG_ALLOCA_WHOLE_IN_MEM__
#define __REG_ALLOCA_WHOLE_IN_MEM__

#include <func.hh>
#include <program.hh>
void whole_in_mem_alloca_func(Func *func, BackEnd *backend);
void whole_in_mem_alloca_program(Program *program);
#endif