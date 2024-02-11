#ifndef __PROGRAM__
#define __PROGRAM__
#include <backend/arch.hh>
#include <backend/backend.hh>
#include <utils.hh>
#include <ast_tree.hh>
class Symbol;
class Func;

class Program {
private:
    Ast_Node *ast_root; // 语法树

    std::vector<Symbol *> symbols; // 全局变量
    std::vector<Func *> funcs; // 函数
    LANG_LEVEL level; // 语言层级（AST、IR、LOWIR、ASM)

    std::string input_file_name; // 输入文件名
    std::string output_file_name; // 输出文件名

    BackEnd *backend; // 后端信息

public:
    Program(std::string input_file, std::string output_file, Arch arch);
    void set_ast_root(Ast_Node *root);
    Ast_Node *get_ast_root();
    void add_symbol(Symbol *symbol);
    void add_func(Func *func);
    std::vector<Symbol *> &get_symbols();
    std::vector<Func *> &get_funcs();
    void set_lang_level(LANG_LEVEL level);
    LANG_LEVEL get_lang_level();

    void print_symbols();
    void print_funcs();

    void set_input_file(std::string name);
    void set_output_file(std::string name);
    std::string get_input_file(std::string name);
    std::string get_output_file(std::string name);
    void print();
    void output_assembler();
    void set_arch(Arch arch);
    Arch get_arch();
    BackEnd *&get_backend();
    ~Program();
};

#endif