#include <backend/backend.hh>
#include <bits/types/FILE.h>
#include <cassert>
#include <cstddef>
#include <func.hh>
#include <program.hh>
#include <symbol.hh>
Program::Program(std::string input_file, std::string output_file, Arch arch) {
    this->input_file_name = input_file;
    this->output_file_name = output_file;
    this->set_arch(arch);
}
void Program::set_ast_root(Ast_Node *root) {
    this->ast_root = root;
}
Ast_Node *Program::get_ast_root() {
    return this->ast_root;
}
void Program::add_symbol(Symbol *symbol) {
    this->symbols.push_back(symbol);
}
void Program::add_func(Func *func) {
    this->funcs.push_back(func);
    func->set_program(this);
}
std::vector<Symbol *> &Program::get_symbols() {
    return this->symbols;
}
std::vector<Func *> &Program::get_funcs() {
    return this->funcs;
}
void Program::set_lang_level(LANG_LEVEL level) {
    this->level = level;
}
LANG_LEVEL Program::get_lang_level() {
    return this->level;
}
void Program::print_symbols() {
    for (auto sym : this->symbols) {
        std::cout << "global ";
        sym->print(level);
        std::cout << std::endl;
    }
}
void Program::print_funcs() {
    for (auto func : this->funcs) {
        func->print(level);
    }
}
void Program::set_input_file(std::string name) {
    this->input_file_name = name;
}
void Program::set_output_file(std::string name) {
    this->output_file_name = name;
}
std::string Program::get_input_file(std::string name) {
    return this->input_file_name;
}
std::string Program::get_output_file(std::string name) {
    return this->output_file_name;
}
void Program::print() {
    this->print_symbols();
    this->print_funcs();
}
void Program::set_arch(Arch arch) {
    switch (arch) {
    case RISCV32:
        this->backend = new RISCV();
        break;
    case ARMv7:
        break;
    }
}
#include <fstream>
static void deal_initializer(std::ofstream &outputFile, Initializer *initializer, Type *type, size_t &zeros, size_t &shl) {
    if (initializer->get_is_val()) {
        if (zeros)
            outputFile << ".zero " << (zeros << shl) << std::endl;
        zeros = 0;
        outputFile << ".word " << initializer->get_init_val().get_i32_val() << std::endl;
        return;
    }
    assert(initializer->get_is_list());
    for (auto initials : initializer->get_initials()) {
        deal_initializer(outputFile, initials, type->get_next_level(), zeros, shl);
    }
    zeros += type->get_size() - initializer->get_initials().size() * type->get_next_level()->get_size();
}
void Program::output_assembler() {
    std::ofstream outputFile(this->output_file_name);
    // 检查文件是否成功打开
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open file!" << std::endl;
        assert(0);
    }
    outputFile << ".file \"" << this->input_file_name << "\"" << std::endl;
    outputFile << ".text" << std::endl;
    outputFile << ".data" << std::endl;
    outputFile << ".align 2 " << std::endl;
    outputFile << std::endl;
    for (auto &sym : symbols) {
        outputFile << ".globl " << sym->get_name() << std::endl;
        outputFile << ".type " << sym->get_name() << ", @object " << std::endl;
        //outputFile << ".size " << sym->get_name() << ", " << sym->get_type()->get_size() << backend->wordsize << std::endl;
        outputFile << sym->get_name() << ":" << std::endl;
        Type *type = sym->get_type();
        Initializer *initializer = sym->get_initials();
        if (!initializer) {
            outputFile << ".zero " << (type->get_size() << backend->wordsize) << std::endl;
            continue;
        }
        size_t zeros = 0;
        deal_initializer(outputFile, initializer, type, zeros, backend->wordsize);
        if (zeros)
            outputFile << ".zero " << (zeros << backend->wordsize) << std::endl;
        outputFile << std::endl;
    }
    for (auto &func : funcs) {
        func->output_assembler(outputFile);
    }
}

BackEnd *&Program::get_backend() {
    return backend;
}
Arch Program::get_arch() {
    return backend->arch;
}
Program::~Program() {
    delete ast_root;
    for (auto sym : this->symbols)
        delete sym;
    for (auto func : this->funcs)
        delete func;
    delete backend;
}