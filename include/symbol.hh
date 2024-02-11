#ifndef __SYMBOL__
#define __SYMBOL__
#include <basic_value.hh>
// class Basic_Value {
// private:
//     Basic_Type basic_type;
//     union {
//         I32CONST_t int_num;
//         F32CONST_t float_num;
//     };

// public:

// };
class Func;
class Program;
class Ast_Node;
class IR_VReg;
class Type;

class Initializer {
private:
    bool is_list;
    std::vector<Initializer *> initials;

    bool is_val;

    Ast_Node *init_exp;
    Basic_Value init_val;

public:
    Initializer(bool is_list);
    Initializer(Ast_Node *init_exp);
    Initializer(Basic_Value init_val);
    // Initializer(Initializer *initializer);
    std::vector<Initializer *> &get_initials();
    void add_initial(Initializer *initial);
    void set_init_val(Basic_Value val);
    Basic_Value get_init_val();
    void set_init_exp(Ast_Node *init_exp);
    Ast_Node *get_init_exp();
    bool get_is_val();
    void set_initial(std::vector<Initializer *> initial);
    bool get_is_list();
    void print();
    ~Initializer();
};
class Symbol {
private:
    bool is_global;
    bool is_const;
    std::string name;
    Type *symbol_type; // 函数类型
    Initializer *initials; // 初始化信息
    size_t depth; // 嵌套深度
    bool is_func_param; // 是否是函数参数
    size_t param_loc; // 若是参数，处于函数的第几个
    union {
        Func *func; // 局部变量指向函数
        Program *program; // 全局变量指向程序
    };

    IR_VReg *addr; // 存放变量地址的虚拟寄存器
public:
    Symbol(std::string name, Type *symbol_type, bool is_global = false, bool is_const = false, size_t depth = 0, bool is_func_param = false, size_t param_loc = 0, Func *func = nullptr, Program *program = nullptr);
    void set_const(bool is_const);
    void set_global(bool is_gloabl);
    void set_depth(int depth);
    void set_func(Func *func);
    bool get_const();
    bool get_global();
    bool get_is_func_param();
    size_t get_param_loc();
    size_t get_depth();
    Initializer *get_initials();
    void set_initial(Initializer *initializer);
    std::string get_name();
    Type *get_type();
    void print(LANG_LEVEL level);

    void set_addr(IR_VReg *p_vreg);
    IR_VReg *get_addr();
    ~Symbol();
};

#endif