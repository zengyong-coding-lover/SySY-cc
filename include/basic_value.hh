#ifndef __BASIC_VALUE__
#define __BASIC_VALUE__
#include <type.hh>
#include <utils.hh>

class Basic_Value {
private:
    Type *type;
    union {
        I32CONST_t int_num;
        F32CONST_t float_num;
    };

public:
    Basic_Value();
    Basic_Value(I32CONST_t int_num);
    Basic_Value(F32CONST_t float_num);
    Basic_Value(const Basic_Value &val);
    I32CONST_t get_i32_val();
    F32CONST_t get_f32_val();
    void print();
    friend std::ostream &operator<<(std::ostream &os, const Basic_Value &val);
    Basic_Type get_basic_type();
    Type *get_type();
    Basic_Value &operator=(const Basic_Value &src);
    Basic_Value operator+(const Basic_Value &src2);
    Basic_Value operator-(const Basic_Value &src2);
    Basic_Value operator*(const Basic_Value &src2);
    Basic_Value operator/(const Basic_Value &src2);
    Basic_Value operator%(const Basic_Value &src2);
    Basic_Value operator<(const Basic_Value &src2);
    Basic_Value operator>(const Basic_Value &src2);
    Basic_Value operator<=(const Basic_Value &src2);
    Basic_Value operator>=(const Basic_Value &src2);
    Basic_Value operator==(const Basic_Value &src2);
    Basic_Value operator!=(const Basic_Value &src2);
    Basic_Value operator||(const Basic_Value &src2);
    Basic_Value operator&&(const Basic_Value &src2);
    Basic_Value operator!();
    Basic_Value operator-();
    ~Basic_Value();
};
#endif