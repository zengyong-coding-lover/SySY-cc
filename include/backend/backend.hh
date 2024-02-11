#ifndef __BACKEND_BACKEND__
#define __BACKEND_BACKEND__
#include <backend/arch.hh>
#include <utils.hh>
typedef size_t REG;
class BackEnd {
private:
public:
    Arch arch;
    size_t reg_num;
    size_t usable_reg_num;
    size_t callee_save_reg_num;
    REG sp;
    REG fp;
    REG ra;
    REG zero;
    REG tmp;
    size_t wordsize;
    BackEnd(Arch arch, size_t reg_num, size_t usable_reg_num, size_t callee_save_reg, size_t sp, size_t fp, size_t ra, size_t zero, size_t tmp, size_t wordsize) {
        this->usable_reg_num = usable_reg_num;
        this->callee_save_reg_num = callee_save_reg;
        this->sp = sp;
        this->fp = fp;
        this->ra = ra;
        this->zero = zero;
        this->tmp = tmp;
        this->wordsize = wordsize;
        this->arch = arch;
    }
    virtual REG update_reg_id(REG reg_id) { return 0; }
    virtual ~BackEnd() { }
    virtual Arch get_arch() { return arch; }
};

#endif