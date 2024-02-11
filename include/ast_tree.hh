#ifndef __AST_TREE__
#define __AST_TREE__
#include <basic_value.hh>
#include <initializer_list>
#include <symbol.hh>
#include <utils.hh>

typedef enum Nonterminal_Node_Type {
    Cond,
    LOrExp,
    LAndExp,
    EqExp,
    RelExp,
    Type_,

    Exp,
    ConstExp,
    AddExp,
    MulExp,
    UnaryExp,
    PrimaryExp,

    Call,
    Val,
    Str,

    StmtExp,
    Stmt,

    BlockItems,
    Block,

    FuncRParamList,
    FuncRParams,

    ArraryParameter,

    Declarator,
    VarInitDeclarator,
    VarInitDeclaratorList,
    ConstInitDeclarator,
    ConstInitDeclaratorList,

    VarInitializer,
    VarInitializerList,
    ConstInitializer,
    ConstInitializerList,

    CompUnit,
    Declaration,
    FuncDeclaration,
    ConstDeclaration,
    VarDeclaration,

    ParameterDeclaration,
    ParameterList,
    FuncHead,
    Parameters,
} Nonterminal_Node_Type;

typedef enum Terminal_Node_Type {
    INT_,
    FLOAT_,
    RETURN_,
    IF_,
    ELSE_,
    WHILE_,
    ID_,
    SPACE_,
    SEMI_,
    COMMA_,
    ASSIGN_,
    // RELOP_,
    PLUS_,
    MINUS_,
    DOT_,
    NOT_,
    LP_,
    RP_,
    LB_,
    RB_,
    LC_,
    RC_,
    CONST_,
    VOID_,
    L_,
    G_,
    LE_,
    GE_,
    EQ_,
    NEQ_,
    ADD_,
    AND_,
    OR_,
    SUB_,
    MUL_,
    DIV_,
    MOD_,
    FOR_,

    STRING_,
    BREAK_,
    CONTINUE_,
    I32CONST_,
    F32CONST_,

    EMPTY_
} Terminal_Node_Type;

class Ast_Node {
private:
    bool is_terminal; // 是否是终端节点
    struct {
        Nonterminal_Node_Type nonterminal_type;
        std::vector<Ast_Node *> childs;
    } nonterminal;
    struct {
        Terminal_Node_Type terminal_type;
        Basic_Value basic_value;

        std::string id_name;
    } terminal;

public:
    // extra info
    // 解析 exp 时是否是 const 信息
    // 当是 declare 时存储声明的symbol
    // (只在 varinitdeclarator 和 constinitdeclarator及ID 用到)
    struct {
        bool is_const;
        Symbol *symbol;
        Basic_Value node_val;
        Func *func;
    } exp_info;

    Ast_Node(Terminal_Node_Type type);
    Ast_Node(Nonterminal_Node_Type type);
    Ast_Node(Terminal_Node_Type type, Basic_Value num);
    Ast_Node(Terminal_Node_Type type, const std::string &id_name);
    Ast_Node(Terminal_Node_Type type, const char *id_name);
    Ast_Node(Nonterminal_Node_Type type, std::initializer_list<Ast_Node *> ast_nodes);
    void set_nontermianl_type(Nonterminal_Node_Type type);
    void set_terminal_type(Terminal_Node_Type type, Basic_Value num);
    void print();
    void Traverse(int depth = 0);
    bool get_is_terminal();
    Nonterminal_Node_Type get_nonterminal_type();
    Terminal_Node_Type get_terminal_type();
    std::string get_id_name();
    Basic_Value &get_basic_value();
    std::vector<Ast_Node *> &get_childs();
    ~Ast_Node();
};
#endif