#include <ast_tree.hh>
#include <basic_value.hh>
#include <func.hh>
#include <program.hh>
#include <string>
#include <symbol.hh>
#include <syntax.hh>
#include <type.hh>
#include <utils.hh>
struct Syntax_Symbol {
    bool is_func;
    int depth;
    union {
        Symbol *sym;
        Func *func;
    };
    Syntax_Symbol(Symbol *sym, int depth) {
        this->is_func = false;
        this->sym = sym;
        this->depth = depth;
    }
    Syntax_Symbol(Func *func) {
        this->is_func = true;
        this->func = func;
        this->depth = 0;
    }
};

static std::map<std::string, std::stack<Syntax_Symbol>> sym_map;
static int depth;
static Program *global_program;
static Func *current_func;

// 定义 symbol 并对语义分析中的信息进行更新
static Symbol *insert_sym_id(std::string id, Type *base_type, bool is_const, bool is_param, size_t loc) {
    auto it = sym_map.find(id);
    bool is_global = current_func == nullptr;
    // 若当前符号表中没有该符号，则新建一个符号，并将其插入符号表中
    if (it == sym_map.end()) {
        std::stack<Syntax_Symbol> new_stack;
        Symbol *new_sym = new Symbol(id, base_type, is_global, is_const, depth, is_param, loc, current_func, global_program);
        new_stack.push(Syntax_Symbol(new_sym, depth));
        sym_map.insert(std::make_pair(id, new_stack));
        return new_sym;
    }
    Syntax_Symbol top_sym = it->second.top();
    // 若当前符号表中有该符号，且该符号的深度与当前深度相同，则说明同一个符号定义了两次，报错
    if (top_sym.depth == depth) {
        ERROR(id + "has declared ");
        exit(0);
    }
    // 若当前符号表中有该符号，且该符号的深度比当前深度小，则说明该符号是在外层定义的，将其插入符号表中
    Symbol *new_sym = new Symbol(id, base_type, is_global, is_const, depth, is_param, loc, current_func, global_program);
    it->second.push(Syntax_Symbol(new_sym, depth));
    return new_sym;
}

// 退出一个块时，清除当前块所在深度中的符号
static void clear_currrent_symbols() {
    auto it = sym_map.begin();
    while (it != sym_map.end()) {
        if (it->second.top().depth == depth) {
            it->second.pop();
        }
        if (it->second.empty())
            it = sym_map.erase(it);
        else
            it++;
    }
}

static Initializer *num_list_initializer(Initializer *initializer, size_t i, size_t j, Type *type) {
    if (type->get_is_basic()) {
        assert(i == j);
        Initializer *init = initializer->get_initials()[i];
        assert(!init->get_is_list());
        if (init->get_is_val())
            return new Initializer(init->get_init_val());
        return new Initializer(init->get_init_exp());
    }
    Initializer *result = new Initializer(true);
    Type *next_level = type->get_next_level();
    for (size_t k = i; k <= j; k += next_level->get_size()) {
        result->add_initial(num_list_initializer(initializer, k, std::min(k + next_level->get_size() - 1, j), next_level));
    }
    return result;
}
// 将initializer 中的数据重新组织成一个新的initializer，主要是连续的数字重组
static Initializer *reset_initializer(Initializer *initializer, Type *type) {
    std::vector<Initializer *> initials = initializer->get_initials();
    size_t length = initials.size();
    size_t i = 0;
    Initializer *result = new Initializer(true);
    Type *next_level = type->get_next_level();
    while (i < length) {
        if (initials[i]->get_is_list())
            result->add_initial(reset_initializer(initials[i++], next_level));
        else {
            size_t j = i;
            while (j < length && !initials[j]->get_is_list()) {
                if ((j - i + 1) % next_level->get_size() == 0) {
                    result->add_initial(num_list_initializer(initializer, i, j, next_level));
                    i = j + 1;
                }
                j++;
            }
            if (i != j) {
                // int a[2][3] ={1,2,3,4,{5}}; 是不允许的
                if (j < length) {
                    ERROR("initializer is wrong");
                    exit(0);
                }
                else
                    result->add_initial(num_list_initializer(initializer, i, j - 1, next_level));
            }
            i = j;
        }
    }
    return result;
}

static void Syntax_CompUnit(Ast_Node *compunit);
static Basic_Type Token_BasicType(Terminal_Node_Type type);
static void Syntax_Declaration(Ast_Node *declaration);
static void Syntax_ConstDecaration(Ast_Node *const_declaration);
static void Syntax_VarDeclaration(Ast_Node *var_declaration);
static void Syntax_ConstInitDeclaratorList(Ast_Node *const_init_declaratorlist);
static void Syntax_VarInitDeclaratorList(Ast_Node *var_init_declaratorlist);
static void Syntax_ConstInitDeclarator(Ast_Node *const_init_declarator, Type *base_type);
static void Syntax_VarInitDeclarator(Ast_Node *var_init_declarator, Type *base_type);
static Symbol *Syntax_Declarator(Ast_Node *declarator, Type *base_type, bool is_const);
static Initializer *Syntax_ConstInitializer(Ast_Node *var_initializer);
static Initializer *Syntax_ConstInitializerList(Ast_Node *var_initial_list);
static Initializer *Syntax_VarInitializer(Ast_Node *var_initializer);
static Initializer *Syntax_VarInitializerList(Ast_Node *var_initial_list);
static Func *Syntax_FuncHead(Ast_Node *func_head);
static void Syntax_FuncDeclaration(Ast_Node *func_declaration);
static void Syntax_Parameters(Ast_Node *parameters);
static void Syntax_ParamterList(Ast_Node *paramter_list, size_t &loc);
static void Syntax_ParameterDeclaration(Ast_Node *parameter_declaration, size_t &loc);
static void Syntax_ArraryParameter(Ast_Node *arrary_parameter, Type *base_type, size_t loc);
static void Syntax_Cond(Ast_Node *cond);
static void Syntax_LOrExp(Ast_Node *lor_exp);
static void Syntax_LAndExp(Ast_Node *land_exp);
static void Syntax_EqExp(Ast_Node *eq_exp);
static void Syntax_RelExp(Ast_Node *rel_exp);
static void Syntax_ConstExp(Ast_Node *const_exp);
static void Syntax_Exp(Ast_Node *exp);
static void Syntax_AddExp(Ast_Node *add_exp);
static void Syntax_MulExp(Ast_Node *mul_exp);
static void Syntax_UnaryExp(Ast_Node *unary_exp);
static void Syntax_PrimaryExp(Ast_Node *primary_exp);

static void Syntax_Call(Ast_Node *call);
static void Syntax_Val(Ast_Node *val);
// static void Syntax_String(Ast_Node *string);
static void Syntax_FuncRParams(Ast_Node *funcrparams, Func *func);
static void Syntax_FuncRParamsList(Ast_Node *funcrparamslist, Func *func);

static void Syntax_Block(Ast_Node *block);
static void Syntax_BlockItems(Ast_Node *block);

static void Syntax_Stmt(Ast_Node *stmt);

/*
CompUnit : CompUnit Declaration 
         | CompUnit FuncDeclaration 
*/
static void Syntax_CompUnit(Ast_Node *compunit) {
    assert(compunit->get_nonterminal_type() == CompUnit);
    if (compunit->get_childs().size() == 1)
        return;
    Syntax_CompUnit(compunit->get_childs()[0]);
    Ast_Node *child2 = compunit->get_childs()[1];
    if (child2->get_nonterminal_type() == Declaration)
        Syntax_Declaration(child2);
    else
        Syntax_FuncDeclaration(child2);
}
/*
Type : INT   
     | FLOAT 
*/
static Basic_Type Token_BasicType(Terminal_Node_Type type) {
    switch (type) {
    case INT_:
        return I32;
    case FLOAT_:
        return F32;
    default:
        assert(0);
    }
}
/*
Declaration : ConstDeclaration 
            | VarDeclaration 
*/
static void Syntax_Declaration(Ast_Node *declaration) {
    assert(declaration->get_nonterminal_type() == Declaration);
    Ast_Node *child = declaration->get_childs()[0];
    if (child->get_nonterminal_type() == ConstDeclaration)
        Syntax_ConstDecaration(child);
    else
        Syntax_VarDeclaration(child);
}

/*
ConstDeclaration : ConstInitDeclaratorList SEMI
*/
static void Syntax_ConstDecaration(Ast_Node *const_declaration) {
    assert(const_declaration->get_nonterminal_type() == ConstDeclaration);
    Syntax_ConstInitDeclaratorList(const_declaration->get_childs()[0]);
}

/*
VarDeclaration : VarInitDeclaratorList SEMI
*/
static void Syntax_VarDeclaration(Ast_Node *var_declaration) {
    assert(var_declaration->get_nonterminal_type() == VarDeclaration);
    Syntax_VarInitDeclaratorList(var_declaration->get_childs()[0]);
}

/* 
ConstInitDeclaratorList : ConstInitDeclaratorList DOT ConstInitDeclarator 
                        | CONST Type ConstInitDeclarator                  
                        ;
*/

static void Syntax_ConstInitDeclaratorList(Ast_Node *const_init_declaration) {
    assert(const_init_declaration->get_nonterminal_type() == ConstInitDeclaratorList);
    std::vector<Ast_Node *> childs = const_init_declaration->get_childs();
    // 获得 CONST Type 的 Type
    while (!childs[0]->get_is_terminal()) {
        assert(childs[0]->get_nonterminal_type() == ConstInitDeclaratorList);
        childs = childs[0]->get_childs();
    }
    assert(childs[1]->get_nonterminal_type() == Type_);
    Ast_Node *type_node = childs[1]->get_childs()[0];

    childs = const_init_declaration->get_childs();
    while (!childs[0]->get_is_terminal()) {
        assert(childs[2]->get_nonterminal_type() == ConstInitDeclarator);
        Type *base_type = new Type(Token_BasicType(type_node->get_terminal_type()));
        Syntax_ConstInitDeclarator(childs[2], base_type);
        childs = childs[0]->get_childs();
    }
    Type *base_type = new Type(Token_BasicType(type_node->get_terminal_type()));
    Syntax_ConstInitDeclarator(childs[2], base_type);
}

/*
VarInitDeclaratorList : VarInitDeclaratorList DOT VarInitDeclarator 
                      | Type VarInitDeclarator
*/
static void Syntax_VarInitDeclaratorList(Ast_Node *var_init_declaratorlist) {
    assert(var_init_declaratorlist->get_nonterminal_type() == VarInitDeclaratorList);
    std::vector<Ast_Node *> childs = var_init_declaratorlist->get_childs();
    // 获得 Type
    while (childs.size() == 3) {
        assert(childs[0]->get_nonterminal_type() == VarInitDeclaratorList);
        childs = childs[0]->get_childs();
    }
    assert(childs[0]->get_nonterminal_type() == Type_);

    // 完成：实现解析以逗号分隔的所有 VarInitDeclarator
    TODO();
    // 实现时请删除以下代码
    Syntax_VarInitDeclarator(nullptr, nullptr);
}

/*
ConstInitDeclarator : Declarator ASSIGN ConstInitializer
*/
static void Syntax_ConstInitDeclarator(Ast_Node *const_init_declarator, Type *base_type) {
    assert(const_init_declarator->get_nonterminal_type() == ConstInitDeclarator);
    Symbol *new_sym = Syntax_Declarator(const_init_declarator->get_childs()[0], base_type, true);
    Initializer *init = Syntax_ConstInitializer(const_init_declarator->get_childs()[2]);
    if (init->get_is_list()) {
        new_sym->set_initial(reset_initializer(init, new_sym->get_type()));
        delete init;
    }
    else
        new_sym->set_initial(init);
    const_init_declarator->exp_info.symbol = new_sym;
}
/*
VarInitDeclarator : Declarator ASSIGN VarInitializer
                  | Declarator                   
*/
static void Syntax_VarInitDeclarator(Ast_Node *var_init_declarator, Type *base_type) {
    assert(var_init_declarator->get_nonterminal_type() == VarInitDeclarator);
    Symbol *new_sym = Syntax_Declarator(var_init_declarator->get_childs()[0], base_type, false);
    if (var_init_declarator->get_childs().size() == 3) {
        Initializer *init = Syntax_VarInitializer(var_init_declarator->get_childs()[2]);
        if (init->get_is_list()) {
            new_sym->set_initial(reset_initializer(init, new_sym->get_type()));
            delete init;
        }
        else
            new_sym->set_initial(init);
    }
    var_init_declarator->exp_info.symbol = new_sym;
}

/*
Declarator : Declarator '[' ConstExp ']'
| ID
*/
/*
输入 节点 declarator, 声明的基础类型 base_type, 以及是否是常量
输出 新的 symbol,
*/
static Symbol *Syntax_Declarator(Ast_Node *declarator, Type *base_type, bool is_const) {
    assert(declarator->get_nonterminal_type() == Declarator);
    while (declarator->get_childs().size() == 4) {
        // 实现：解析数组声明得到数据类型
        // 提示：const_exp.exp_info.node_val.get_i32_val() 已经计算得到const_exp的整型值
        TODO();
        declarator = declarator->get_childs()[0];
    }
    std::string id = declarator->get_childs()[0]->get_id_name();
    Symbol *sym = insert_sym_id(id, base_type, is_const, false, 0);
    declarator->get_childs()[0]->exp_info.symbol = sym;
    return sym;
}

/*
ConstInitializer : LB ConstInitializerList RB    
                 | LB RB                         
                 | ConstExp             
                 ;
*/
static Initializer *Syntax_ConstInitializer(Ast_Node *const_initializer) {
    assert(const_initializer->get_nonterminal_type() == ConstInitializer);
    if (const_initializer->get_childs().size() == 1) {
        Ast_Node *const_exp = const_initializer->get_childs()[0];
        Syntax_ConstExp(const_exp);
        return new Initializer(const_exp->exp_info.node_val);
    }
    if (const_initializer->get_childs().size() == 2) {
        return new Initializer(true);
    }
    assert(const_initializer->get_childs().size() == 3);
    return Syntax_ConstInitializerList(const_initializer->get_childs()[1]);
}

/*
ConstInitializerList : ConstInitializerList DOT ConstInitializer
                     | ConstInitializer                         
                     ;
*/
static Initializer *Syntax_ConstInitializerList(Ast_Node *const_initial_list) {
    assert(const_initial_list->get_nonterminal_type() == ConstInitializerList);
    if (const_initial_list->get_childs().size() == 3) {
        Initializer *new_initial = Syntax_ConstInitializerList(const_initial_list->get_childs()[0]);
        new_initial->add_initial(Syntax_ConstInitializer(const_initial_list->get_childs()[2]));
        return new_initial;
    }
    Initializer *new_initial_list = new Initializer(true);
    new_initial_list->add_initial(Syntax_ConstInitializer(const_initial_list->get_childs()[0]));
    return new_initial_list;
}

/*
VarInitializer : LB VarInitializerList RB   
               | LB RB                       
               | Exp                    
               ;
*/
static Initializer *Syntax_VarInitializer(Ast_Node *var_initializer) {
    assert(var_initializer->get_nonterminal_type() == VarInitializer);
    if (var_initializer->get_childs().size() == 1) {
        Ast_Node *exp = var_initializer->get_childs()[0];
        Syntax_Exp(exp);
        if (exp->exp_info.is_const)
            return new Initializer(exp->exp_info.node_val);
        return new Initializer(exp);
    }
    if (var_initializer->get_childs().size() == 2) {
        return new Initializer(true);
    }
    assert(var_initializer->get_childs().size() == 3);
    return Syntax_VarInitializerList(var_initializer->get_childs()[1]);
}

/*
VarInitializerList : VarInitializerList DOT VarInitializer
                   | VarInitializer                       
                   ;
*/
static Initializer *Syntax_VarInitializerList(Ast_Node *var_initial_list) {
    assert(var_initial_list->get_nonterminal_type() == VarInitializerList);
    if (var_initial_list->get_childs().size() == 3) {
        Initializer *new_initial = Syntax_VarInitializerList(var_initial_list->get_childs()[0]);
        new_initial->add_initial(Syntax_VarInitializer(var_initial_list->get_childs()[2]));
        return new_initial;
    }
    Initializer *new_initial_list = new Initializer(true);
    new_initial_list->add_initial((Syntax_VarInitializer(var_initial_list->get_childs()[0])));
    return new_initial_list;
}

/*
    FuncHead : Type ID 
    | VOID ID 
*/
static Func *Syntax_FuncHead(Ast_Node *funchead) {
    assert(funchead->get_nonterminal_type() == FuncHead);
    assert(current_func == nullptr);
    assert(depth == 0);
    std::string id_name = funchead->get_childs()[1]->get_id_name();
    if (sym_map.find(id_name) != sym_map.end()) {
        ERROR(id_name + " has been  declared");
        exit(0);
    }
    Func *func;
    // 实现： 解析函数头部，根据返回值类型创建函数对象
    TODO();
    global_program->add_func(func);
    std::stack<Syntax_Symbol> new_stack;
    new_stack.push(Syntax_Symbol(func));
    sym_map.insert(std::make_pair(id_name, new_stack));
    return func;
}

/*
FuncDeclaration : FuncHead LP Parameters RP Block
*/
static void Syntax_FuncDeclaration(Ast_Node *funcdeclaration) {
    assert(funcdeclaration->get_nonterminal_type() == FuncDeclaration);
    current_func = Syntax_FuncHead(funcdeclaration->get_childs()[0]);
    depth++;
    Syntax_Parameters(funcdeclaration->get_childs()[2]);
    Syntax_Block(funcdeclaration->get_childs()[4]);
    current_func->set_ast_body(funcdeclaration->get_childs()[4]);
    current_func = nullptr;
    depth--;
}

/*
Parameters : ParameterList
           | empty 
*/
static void Syntax_Parameters(Ast_Node *parameters) {
    assert(parameters->get_nonterminal_type() == Parameters);
    if (!parameters->get_childs()[0]->get_is_terminal()) {
        size_t loc = 0;
        Syntax_ParamterList(parameters->get_childs()[0], loc);
    }
}

/*
ParameterList : ParameterList DOT ParameterDeclaration 
              | ParameterDeclaration 
              ;
*/
static void Syntax_ParamterList(Ast_Node *paramter_list, size_t &loc) {
    assert(paramter_list->get_nonterminal_type() == ParameterList);
    if (paramter_list->get_childs().size() == 1) {
        Syntax_ParameterDeclaration(paramter_list->get_childs()[0], loc);
        return;
    }
    Syntax_ParamterList(paramter_list->get_childs()[0], loc);
    Syntax_ParameterDeclaration(paramter_list->get_childs()[2], loc);
}
/*
ParameterDeclaration : Type ArraryParameter 
                     | Type ID                                   ;
*/
static void Syntax_ParameterDeclaration(Ast_Node *parameter_declaration, size_t &loc) {
    assert(parameter_declaration->get_nonterminal_type() == ParameterDeclaration);
    Type *base_type = new Type(Token_BasicType(parameter_declaration->get_childs()[0]->get_childs()[0]->get_terminal_type()));
    Ast_Node *child1 = parameter_declaration->get_childs()[1];
    if (child1->get_is_terminal()) {
        insert_sym_id(child1->get_id_name(), base_type, false, true, loc);
        loc++;
        return;
    }
    Syntax_ArraryParameter(child1, base_type, loc);
    loc++;
}

/*
ArraryParameter : ID LC RC                 
                | ArraryParameter LC Exp RC
*/
static void Syntax_ArraryParameter(Ast_Node *arrary_parameter, Type *base_type, size_t loc) {
    assert(arrary_parameter->get_nonterminal_type() == ArraryParameter);
    Ast_Node *child1 = arrary_parameter->get_childs()[0];
    if (child1->get_is_terminal()) {
        base_type->ptr_level_inc();
        insert_sym_id(child1->get_id_name(), base_type, false, true, loc);
        return;
    }
    Ast_Node *const_exp = arrary_parameter->get_childs()[2];
    Syntax_ConstExp(const_exp);
    base_type = new Type(base_type, (size_t) const_exp->exp_info.node_val.get_i32_val(), 0);
    Syntax_ArraryParameter(child1, base_type, loc);
}

/*
Cond : LOrExp
*/
static void Syntax_Cond(Ast_Node *cond) {
    assert(cond->get_nonterminal_type() == Cond);
    Ast_Node *lor_exp = cond->get_childs()[0];
    Syntax_LOrExp(lor_exp);
    cond->exp_info = lor_exp->exp_info;
}

/*
LOrExp : LOrExp OR LAndExp 
       | LAndExp 
*/
static void Syntax_LOrExp(Ast_Node *lor_exp) {
    assert(lor_exp->get_nonterminal_type() == LOrExp);
    if (lor_exp->get_childs().size() == 1) {
        Ast_Node *land_exp = lor_exp->get_childs()[0];
        Syntax_LAndExp(land_exp);
        lor_exp->exp_info = land_exp->exp_info;
        return;
    }
    Ast_Node *lor_exp_child = lor_exp->get_childs()[0];
    Ast_Node *land_exp = lor_exp->get_childs()[2];
    Syntax_LOrExp(lor_exp_child);
    Syntax_LAndExp(land_exp);
    if (land_exp->exp_info.is_const & lor_exp_child->exp_info.is_const) {
        lor_exp->exp_info.is_const = true;
        lor_exp->exp_info.node_val = land_exp->exp_info.node_val || lor_exp_child->exp_info.node_val;
        return;
    }
    lor_exp->exp_info.is_const = false;
}

/*
LAndExp : LAndExp AND EqExp 
        | EqExp             
*/
static void Syntax_LAndExp(Ast_Node *land_exp) {
    assert(land_exp->get_nonterminal_type() == LAndExp);
    if (land_exp->get_childs().size() == 1) {
        Ast_Node *eq_exp = land_exp->get_childs()[0];
        Syntax_EqExp(eq_exp);
        land_exp->exp_info = eq_exp->exp_info;
        return;
    }
    Ast_Node *land_exp_child = land_exp->get_childs()[0];
    Ast_Node *eq_exp = land_exp->get_childs()[2];
    Syntax_LAndExp(land_exp_child);
    Syntax_EqExp(eq_exp);
    if (land_exp_child->exp_info.is_const & eq_exp->exp_info.is_const) {
        land_exp->exp_info.is_const = true;
        land_exp->exp_info.node_val = land_exp_child->exp_info.node_val && eq_exp->exp_info.node_val;
        return;
    }
    land_exp->exp_info.is_const = false;
}

/*
EqExp : EqExp EQ RelExp 
      | EqExp NEQ RelExp
      | RelExp 
*/
static void Syntax_EqExp(Ast_Node *eq_exp) {
    assert(eq_exp->get_nonterminal_type() == EqExp);
    if (eq_exp->get_childs().size() == 1) {
        Ast_Node *rel_exp = eq_exp->get_childs()[0];
        Syntax_RelExp(rel_exp);
        eq_exp->exp_info = rel_exp->exp_info;
        return;
    }
    Ast_Node *eq_exp_child = eq_exp->get_childs()[0];
    Ast_Node *rel_exp = eq_exp->get_childs()[2];
    Syntax_EqExp(eq_exp_child);
    Syntax_RelExp(rel_exp);
    Ast_Node *op = eq_exp->get_childs()[1];
    if (eq_exp_child->exp_info.is_const & rel_exp->exp_info.is_const) {
        eq_exp->exp_info.is_const = true;
        switch (op->get_terminal_type()) {
        case EQ_:
            eq_exp->exp_info.node_val = eq_exp_child->exp_info.node_val == rel_exp->exp_info.node_val;
            break;
        case NEQ_:
            eq_exp->exp_info.node_val = eq_exp_child->exp_info.node_val != rel_exp->exp_info.node_val;
            break;
        default:
            assert(0);
        }
        return;
    }
    eq_exp->exp_info.is_const = false;
}
/*
RelExp : RelExp L AddExp 
       | RelExp G AddExp 
       | RelExp LE AddExp 
       | RelExp GE AddExp 
       | AddExp 
*/
static void Syntax_RelExp(Ast_Node *rel_exp) {
    assert(rel_exp->get_nonterminal_type() == RelExp);
    if (rel_exp->get_childs().size() == 1) {
        Ast_Node *add_exp = rel_exp->get_childs()[0];
        Syntax_AddExp(add_exp);
        rel_exp->exp_info = add_exp->exp_info;
        return;
    }
    Ast_Node *rel_exp_child = rel_exp->get_childs()[0];
    Ast_Node *add_exp = rel_exp->get_childs()[2];
    Syntax_RelExp(rel_exp_child);
    Syntax_AddExp(add_exp);
    Ast_Node *op = rel_exp->get_childs()[1];
    if (rel_exp_child->exp_info.is_const & add_exp->exp_info.is_const) {
        rel_exp->exp_info.is_const = true;
        switch (op->get_terminal_type()) {
        case L_:
            rel_exp->exp_info.node_val = rel_exp_child->exp_info.node_val < add_exp->exp_info.node_val;
            break;
        case G_:
            rel_exp->exp_info.node_val = rel_exp_child->exp_info.node_val > add_exp->exp_info.node_val;
            break;
        case LE_:
            rel_exp->exp_info.node_val = rel_exp_child->exp_info.node_val <= add_exp->exp_info.node_val;
            break;
        case GE_:
            rel_exp->exp_info.node_val = rel_exp_child->exp_info.node_val >= add_exp->exp_info.node_val;
            break;
        default:
            assert(0);
        }
        return;
    }
    rel_exp->exp_info.is_const = false;
}

/*
ConstExp : Exp
*/
static void Syntax_ConstExp(Ast_Node *constexp) {
    assert(constexp->get_nonterminal_type() == ConstExp);
    Ast_Node *exp = constexp->get_childs()[0];
    Syntax_Exp(exp);
    if (!exp->exp_info.is_const) {
        ERROR("must need be constexp!");
        exit(0);
    }
    constexp->exp_info = exp->exp_info;
}
/*
Exp : AddExp 
*/

static void Syntax_Exp(Ast_Node *exp) {
    assert(exp->get_nonterminal_type() == Exp);
    Ast_Node *add_exp = exp->get_childs()[0];
    Syntax_AddExp(add_exp);
    exp->exp_info = add_exp->exp_info;
}

/*
AddExp : AddExp ADD MulExp
       | AddExp SUB MulExp
       | MulExp 
       ;
*/
static void Syntax_AddExp(Ast_Node *add_exp) {
    assert(add_exp->get_nonterminal_type() == AddExp);
    // AddExp -> MulExp
    if (add_exp->get_childs().size() == 1) {
        Ast_Node *mul_exp = add_exp->get_childs()[0];
        Syntax_MulExp(mul_exp);
        add_exp->exp_info = mul_exp->exp_info;
        return;
    }
    assert(add_exp->get_childs().size() == 3);
    Ast_Node *child1 = add_exp->get_childs()[0];
    Ast_Node *child2 = add_exp->get_childs()[2];
    Syntax_AddExp(child1);
    Syntax_MulExp(child2);
    Ast_Node *op = add_exp->get_childs()[1];
    if (child1->exp_info.is_const & child2->exp_info.is_const) {
        add_exp->exp_info.is_const = true;
        switch (op->get_terminal_type()) {
        case ADD_:
            add_exp->exp_info.node_val = child1->exp_info.node_val + child2->exp_info.node_val;
            break;
        case SUB_:
            add_exp->exp_info.node_val = child1->exp_info.node_val - child2->exp_info.node_val;
            break;
        default:
            assert(0);
        }
        return;
    }
    add_exp->exp_info.is_const = false;
}

/*
MulExp : MulExp MUL UnaryExp 
| MulExp DIV UnaryExp
| MulExp MOD UnaryExp 
| UnaryExp 
*/
static void Syntax_MulExp(Ast_Node *mul_exp) {
    assert(mul_exp->get_nonterminal_type() == MulExp);
    if (mul_exp->get_childs().size() == 1) {
        Ast_Node *unary_exp = mul_exp->get_childs()[0];
        Syntax_UnaryExp(unary_exp);
        mul_exp->exp_info = unary_exp->exp_info;
        return;
    }
    assert(mul_exp->get_childs().size() == 3);
    Ast_Node *child1 = mul_exp->get_childs()[0];
    Ast_Node *child2 = mul_exp->get_childs()[2];
    Syntax_MulExp(child1);
    Syntax_UnaryExp(child2);
    Ast_Node *op = mul_exp->get_childs()[1];
    if (child1->exp_info.is_const & child2->exp_info.is_const) {
        mul_exp->exp_info.is_const = true;
        switch (op->get_terminal_type()) {
        case MUL_:
            mul_exp->exp_info.node_val = child1->exp_info.node_val * child2->exp_info.node_val;
            break;
        case DIV_:
            mul_exp->exp_info.node_val = child1->exp_info.node_val / child2->exp_info.node_val;
            break;
        case MOD_:
            mul_exp->exp_info.node_val = child1->exp_info.node_val % child2->exp_info.node_val;
            break;
        default:
            assert(0);
        }
    }
}

/*
UnaryExp : SUB UnaryExp 
         | ADD UnaryExp     
         | NOT UnaryExp    
         | PrimaryExp       
*/

static void Syntax_UnaryExp(Ast_Node *unary_exp) {
    assert(unary_exp->get_nonterminal_type() == UnaryExp);
    if (unary_exp->get_childs().size() == 1) {
        Ast_Node *primary_exp = unary_exp->get_childs()[0];
        Syntax_PrimaryExp(primary_exp);
        unary_exp->exp_info = primary_exp->exp_info;
        return;
    }
    assert(unary_exp->get_childs().size() == 2);
    Ast_Node *child = unary_exp->get_childs()[1];
    Syntax_UnaryExp(child);
    Ast_Node *op = unary_exp->get_childs()[0];
    if (child->exp_info.is_const) {
        unary_exp->exp_info.is_const = true;
        switch (op->get_terminal_type()) {
        case SUB_:
            unary_exp->exp_info.node_val = -child->exp_info.node_val;
            break;
        case ADD_:
            unary_exp->exp_info.node_val = child->exp_info.node_val;
            break;
        case NOT_:
            unary_exp->exp_info.node_val = !child->exp_info.node_val;
            break;
        default:
            assert(0);
        }
    }
}

/*
PrimaryExp : LP Exp RP 
           | I32CONST  
           | F32CONST  
           | Val       
           | Call      
           | Str       
           ;
*/
static void Syntax_PrimaryExp(Ast_Node *primary_exp) {
    assert(primary_exp->get_nonterminal_type() == PrimaryExp);
    if (primary_exp->get_childs().size() == 3) {
        Ast_Node *exp = primary_exp->get_childs()[1];
        Syntax_Exp(exp);
        primary_exp->exp_info = exp->exp_info;
        return;
    }
    Ast_Node *child = primary_exp->get_childs()[0];
    if (child->get_is_terminal()) {
        primary_exp->exp_info.is_const = true;
        primary_exp->exp_info.node_val = child->get_basic_value();
        return;
    }
    switch (child->get_nonterminal_type()) {
    case Val:
        Syntax_Val(child);
        primary_exp->exp_info = child->exp_info;
        break;
    case Call:
        Syntax_Call(child);
        primary_exp->exp_info.is_const = false;
        break;
    default:
        assert(0);
    }
}

/*
Call : ID LP FuncRParams RP 
*/
static void Syntax_Call(Ast_Node *call) {
    assert(call->get_nonterminal_type() == Call);
    std::string id = call->get_childs()[0]->get_id_name();
    auto it = sym_map.find(id);
    if (it == sym_map.end()) {
        ERROR(id + " hasn't been declared");
        exit(0);
    }
    if (!it->second.top().is_func) {
        ERROR(id + "isn't a func");
        exit(0);
    }
    Syntax_FuncRParams(call->get_childs()[2], it->second.top().func);
    call->get_childs()[0]->exp_info.func = it->second.top().func;
}

/*
Val : ID               
    | Val LC Exp RC    
*/
static void Syntax_Val(Ast_Node *val) {
    assert(val->get_nonterminal_type() == Val);
    std::vector<Basic_Value> index_list;
    bool index_is_const = true;
    Ast_Node *child_val = val;
    while (child_val->get_childs().size() == 4) {
        Ast_Node *exp = child_val->get_childs()[2];
        Syntax_Exp(exp);
        if (exp->exp_info.is_const)
            index_list.push_back(exp->exp_info.node_val);
        else
            index_is_const = false;
        child_val = child_val->get_childs()[0];
    }
    assert(child_val->get_childs().size() == 1);
    std::string id = child_val->get_childs()[0]->get_id_name();
    auto it = sym_map.find(id);
    if (it == sym_map.end()) {
        ERROR(id + " has not defined");
        exit(0);
    }
    Syntax_Symbol syntax_sym = it->second.top();
    if (syntax_sym.is_func) {
        ERROR(id + " has been declared a func name");
        exit(0);
    }
    Symbol *sym = syntax_sym.sym;
    // if (sym->get_type()->get_level() != index_list.size()) {
    //     ERROR(sym->get_name() + "index is not an element");
    //     exit(0);
    // }
    // const 数组在下标为常数时 可以直接获得值
    if (sym->get_const() & index_is_const) {
        val->exp_info.is_const = true;
        // 不断索引
        Initializer *init = sym->get_initials();
        for (Basic_Value index : index_list) {
            if ((size_t) index.get_i32_val() < init->get_initials().size())
                init = init->get_initials()[index.get_i32_val()];
            else
                break;
        }
        if (!init->get_is_list())
            val->exp_info.node_val = init->get_init_val();
        else
            val->exp_info.node_val = Basic_Value(I32CONST_t(0));
    }
    assert(child_val->get_childs()[0]->get_terminal_type() == ID_);
    assert(sym);
    child_val->get_childs()[0]->exp_info.symbol = sym; // id 指向 sym
}

// static void Syntax_String(Ast_Node *string) {
// }

/*
FuncRParams : FuncRParamList 
            | empty 
*/

static void Syntax_FuncRParams(Ast_Node *funcrparams, Func *func) {
    assert(funcrparams->get_nonterminal_type() == FuncRParams);
    Ast_Node *child = funcrparams->get_childs()[0];
    if (child->get_is_terminal() && child->get_terminal_type() == EMPTY_) {
        if (func->get_params().size() != 0) {
            ERROR(func->get_name() + " has wrong params");
            exit(0);
        }
        return;
    }
    Syntax_FuncRParamsList(funcrparams->get_childs()[0], func);
}

/*
FuncRParamList : FuncRParamList DOT Exp 
               | Exp                    
*/
static void Syntax_FuncRParamsList(Ast_Node *funcrparamslist, Func *func) {
    assert(funcrparamslist->get_nonterminal_type() == FuncRParamList);
    size_t num = 0;
    while (funcrparamslist->get_childs().size() == 3) {
        Syntax_Exp(funcrparamslist->get_childs()[2]);
        num++;
        funcrparamslist = funcrparamslist->get_childs()[0];
    }
    num++;
    Syntax_Exp(funcrparamslist->get_childs()[0]);
    if (func->get_params().size() != num) {
        ERROR(func->get_name() + " has wrong params");
        exit(0);
    }
}

/*
Block : LB BlockItems RB
*/
static void Syntax_Block(Ast_Node *block) {
    assert(block->get_nonterminal_type() == Block);
    Syntax_BlockItems(block->get_childs()[1]);
    clear_currrent_symbols();
}

/*
BlockItems : BlockItems Declaration 
           | BlockItems Stmt          
           | empty 
*/
static void Syntax_BlockItems(Ast_Node *block_items) {
    assert(block_items->get_nonterminal_type() == BlockItems);
    if (block_items->get_childs().size() == 1) return;
    Syntax_BlockItems(block_items->get_childs()[0]);
    if (block_items->get_childs()[1]->get_nonterminal_type() == Declaration)
        Syntax_Declaration(block_items->get_childs()[1]);
    else
        Syntax_Stmt(block_items->get_childs()[1]);
}

/*
StmtExp : empty 
| Exp 
*/
static void Syntax_StmtExp(Ast_Node *stmt_exp) {
    assert(stmt_exp->get_nonterminal_type() == StmtExp);
    if (!stmt_exp->get_childs()[0]->get_is_terminal()){
        Ast_Node *exp = stmt_exp->get_childs()[0];
        Syntax_Exp(exp);
        stmt_exp->exp_info = exp->exp_info;
    }
}
/*
Stmt : Block             
     | Val ASSIGN Exp SEMI              
     | StmtExp SEMI                     
     | RETURN StmtExp SEMI              
     | BREAK SEMI                       
     | CONTINUE SEMI                    
     | IF LP Cond RP Stmt ELSE Stmt     
     | IF LP Cond RP Stmt %prec NO_ELSE 
     | WHILE LP Cond RP Stmt                   
*/
static void Syntax_Stmt(Ast_Node *stmt) {
    assert(stmt->get_nonterminal_type() == Stmt);
    for (auto &child : stmt->get_childs()) {
        if (child->get_is_terminal()) continue;
        switch (child->get_nonterminal_type()) {
        case Block:
            depth++;
            Syntax_Block(child);
            depth--;
            break;
        case Val:
            Syntax_Val(child);
            break;
        case Exp:
            Syntax_Exp(child);
            break;
        case StmtExp:
            Syntax_StmtExp(child);
            break;
        case Cond:
            Syntax_Cond(child);
            break;
        case Stmt:
            Syntax_Stmt(child);
            break;
        default:
            assert(0);
        }
    }
}
Func *new_lib_func(std::string name, Type *type) {
    Func *func;
    if (!type)
        func = new Func(name, true, true);
    else
        func = new Func(name, type, true);
    std::stack<Syntax_Symbol> new_stack;
    new_stack.push(Syntax_Symbol(func));
    sym_map.insert(std::make_pair(name, new_stack));
    return func;
}
static void add_lib_funcs(Program *program) {
    Func *get_int = new_lib_func("getint", new Type(I32));
    program->add_func(get_int);

    Func *get_ch = new_lib_func("getch", new Type(I32));
    program->add_func(get_ch);

    Func *get_float = new_lib_func("getfloat", new Type(F32));
    program->add_func(get_float);

    Func *get_array = new_lib_func("getarray", new Type(I32));
    new Symbol("a", new Type(I32, 1), false, false, 1, true, 0, get_array);
    get_array->add_param_vreg(new IR_VReg(new Type(I32, 1), 0));
    program->add_func(get_array);

    Func *get_farray = new_lib_func("getfarray", new Type(I32));
    new Symbol("a", new Type(F32, 1), false, false, 1, true, 0, get_farray);
    get_farray->add_param_vreg(new IR_VReg(new Type(F32, 1), 0));
    program->add_func(get_farray);

    Func *put_int = new_lib_func("putint", nullptr);
    new Symbol("a", new Type(I32), false, false, 1, true, 0, put_int);
    put_int->add_param_vreg(new IR_VReg(new Type(I32), 0));
    program->add_func(put_int);

    Func *put_ch = new_lib_func("putch", nullptr);
    new Symbol("a", new Type(I32), false, false, 1, true, 0, put_ch);
    put_ch->add_param_vreg(new IR_VReg(new Type(I32), 0));
    program->add_func(put_ch);

    Func *put_array = new_lib_func("putarray", nullptr);
    new Symbol("n", new Type(I32), false, false, 1, true, 0, put_array);
    new Symbol("a", new Type(I32, 1), false, false, 1, true, 1, put_array);
    put_array->add_param_vreg(new IR_VReg(new Type(I32), 0));
    put_array->add_param_vreg(new IR_VReg(new Type(I32, 1), 1));
    program->add_func(put_array);

    Func *put_float = new_lib_func("putfloat", nullptr);
    new Symbol("a", new Type(F32), false, false, 1, true, 0, put_float);
    put_float->add_param_vreg(new IR_VReg(new Type(F32), 0));
    program->add_func(put_float);

    Func *put_farray = new_lib_func("putfarray", nullptr);
    new Symbol("n", new Type(I32), false, false, 1, true, 0, put_farray);
    new Symbol("a", new Type(F32, 1), false, false, 1, true, 1, put_farray);
    put_farray->add_param_vreg(new IR_VReg(new Type(I32), 0));
    put_farray->add_param_vreg(new IR_VReg(new Type(F32, 1), 1));
    program->add_func(put_farray);
}
void Syntax(Program *program) {
    global_program = program;
    add_lib_funcs(program);
    Syntax_CompUnit(program->get_ast_root());
    assert(depth == 0);
    clear_currrent_symbols();
    program->set_lang_level(AST);
}