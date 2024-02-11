#include <func.hh>
#include <program.hh>
#include <symbol.hh>
#include <type.hh>
Symbol::Symbol(std::string name, Type *symbol_type, bool is_global, bool is_const, size_t depth, bool is_func_param, size_t param_loc, Func *func, Program *program) {
    assert(!is_global | !is_func_param);
    this->is_global = is_global;
    this->name = name;
    this->symbol_type = symbol_type;
    this->is_const = is_const;
    this->depth = depth;
    this->is_func_param = is_func_param;
    this->param_loc = param_loc;
    if (is_global) {
        this->program = program;
        program->add_symbol(this);
        this->symbol_type->set_ptr_level(this->symbol_type->get_ptr_level() + 1);
    }
    else if (is_func_param) {
        assert(func->get_params().size() == param_loc);
        this->func = func;
        this->param_loc = param_loc;
        func->add_param(this);
    }
    else {
        this->func = func;
        func->add_symbol(this);
    }
    this->initials = nullptr;
}
void Symbol::set_const(bool is_const) {
    this->is_const = is_const;
}
void Symbol::set_global(bool is_gloabl) {
    this->is_global = is_gloabl;
}
void Symbol::set_depth(int depth) {
    this->depth = depth;
}
void Symbol::set_func(Func *func) {
    this->func = func;
}
bool Symbol::get_const() {
    return this->is_const;
}
bool Symbol::get_global() {
    return this->is_global;
}
bool Symbol::get_is_func_param() {
    return this->is_func_param;
}
size_t Symbol::get_param_loc() {
    return this->param_loc;
}
size_t Symbol::get_depth() {
    return this->depth;
}
Initializer *Symbol::get_initials() {
    return this->initials;
}
void Symbol::set_initial(Initializer *initializer) {
    this->initials = initializer;
}
std::string Symbol::get_name() {
    return this->name;
}
Type *Symbol::get_type() {
    return this->symbol_type;
}
void Symbol::print(LANG_LEVEL level) {
    if (level == AST) {
        std::cout << "name:" << this->name;
        if (this->is_global) {
            std::cout << "     kind:global  ";
        }
        else {
            if (this->is_func_param)
                std::cout << std::setw(20) << "     kind:param(" + this->func->get_name() + ")";
            else
                std::cout << std::setw(20) << "     kind:local(" + this->func->get_name() + ")";
        }
        if (this->is_const)
            std::cout << std::setw(10) << "     const/var:const";
        else
            std::cout << std::setw(10) << "     const/var:var";
        std::cout << std::setw(5) << "      depth:" << depth;
        std::cout << "      type:";
        this->symbol_type->print();
        std::cout << "      initials:";
        if (this->initials)
            this->initials->print();
        else
            std::cout << "not initializer";
        return;
    }
    if (level == IR) {
        this->symbol_type->print();
        if (this->is_global) {
            std::cout << " @" << this->name << " ";
            return;
        }
        if (this->is_func_param) {
            std::cout << " #" << this->name << " ";
            return;
        }
        std::cout << " $" << this->name << " ";
    }
    if (level == LOWIR) {
        if (this->is_global)
            std::cout << " @" << this->name;
        else
            std::cout << " " << this->symbol_type->get_size();
    }
}

void Symbol::set_addr(IR_VReg *p_vreg) {
    this->addr = p_vreg;
}
IR_VReg *Symbol::get_addr() {
    return this->addr;
}
Symbol::~Symbol() {
    delete symbol_type;
    delete initials;
}

Initializer::Initializer(bool is_list) {
    this->is_list = is_list;
}
Initializer::Initializer(Ast_Node *init_exp) {
    this->is_list = false;
    this->init_exp = init_exp;
    this->is_val = false;
}
Initializer::Initializer(Basic_Value val) {
    this->is_list = false;
    this->init_val = val;
    this->is_val = true;
}
// Initializer::Initializer(Initializer *initializer) {
//     initializer->print();
//     assert(!initializer->is_list);
//     this->is_list = initializer->is_list;
//     this->is_num = initializer->is_num;
//     if (this->is_num)
//         this->init_num = initializer->init_num;
//     else
//         this->init_exp = initializer->init_exp;
// }

std::vector<Initializer *> &Initializer::get_initials() {
    assert(this->is_list);
    return this->initials;
}
Basic_Value Initializer::get_init_val() {
    assert(!this->is_list);
    assert(this->is_val);
    return this->init_val;
}
bool Initializer::get_is_val() {
    return this->is_val;
}
void Initializer::add_initial(Initializer *initial) {
    assert(this->is_list);
    this->initials.push_back(initial);
}
void Initializer::set_init_val(Basic_Value val) {
    this->is_list = false;
    this->is_val = true;
    this->init_val = val;
}
void Initializer::set_init_exp(Ast_Node *init_exp) {
    this->is_list = false;
    this->init_exp = init_exp;
}
Ast_Node *Initializer::get_init_exp() {
    assert(!this->is_list);
    assert(!this->is_val);
    return this->init_exp;
}
void Initializer::set_initial(std::vector<Initializer *> initial) {
    this->is_list = true;
    this->initials = initial;
}
void Initializer::print() {
    if (this->is_list) {
        if (this->initials.size() == 0) {
            std::cout << "{}";
            return;
        }
        std::cout << "{";
        this->initials[0]->print();
        for (size_t i = 1; i < this->initials.size(); i++) {
            std::cout << ", ";
            this->initials[i]->print();
        }
        std::cout << "}";
        return;
    }
    if (this->is_val)
        std::cout << std::setw(5) << this->init_val;
    else
        std::cout << std::setw(5) << "exp";
}
bool Initializer::get_is_list() {
    return this->is_list;
}
Initializer::~Initializer() {
    if (is_list)
        for (auto init : this->initials)
            delete init;
}