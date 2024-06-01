#include <ast2ir.hh>
#include <parser.hh>
#include <passmanager.hh>
#include <program.hh>
#include <syntax.hh>
#include <utils.hh>
void compiler(std::string input_file, std::string output_file) {
    std::cout << " + TEST " << input_file << std::endl;

    Program program = Program(input_file, output_file, RISCV32);
    PassManager passes(&program);

    std::cout << "Test_0 Lexer& Parser" << std::endl;
    parser(input_file, program);
    program.get_ast_root()->Traverse();

    std::cout << "Test_1 syntax(get sym table)" << std::endl;
    Syntax(&program);
    program.print();

    std::cout << "Test_2 ast2ir" << std::endl;
    passes.add_pass(AST2IR);
    passes.add_pass(SIMPLIFY_CFG);
    passes.run();
    program.print();
    passes.add_pass(SIMPLIFY_CFG);
    passes.add_pass(MEM2REG);
    passes.run();
    std::cout << "Test_2 mem2reg" << std::endl;
    program.print();

    std::cout << "Test_3 reg_alloca" << std::endl;
    passes.add_pass(LOWER_IR);
    passes.add_pass(STANDRAD_LOWIR);
    passes.add_pass(WHOLE_IN_MEM_ALLOCA);
    passes.add_pass(AFTER_ALLOCA);
    passes.run();
    program.print();

    std::cout << "Test_4 ir_asm" << std::endl;
    passes.add_pass(LOWIR_ASM);
    passes.run();
    std::cout << "finish" << std::endl;
    program.output_assembler();
}

int main(int argc, const char *argv[]) {
    for (int i = 1; i < argc; i += 2) {
        compiler(argv[i], argv[i + 1]);
    }
    return 0;
}
