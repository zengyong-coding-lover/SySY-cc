#include <ast_tree.hh>

Ast_Node::Ast_Node(Terminal_Node_Type type) {
    this->is_terminal = true;
    this->terminal.terminal_type = type;
    this->exp_info = { false, nullptr, Basic_Value() };
}
Ast_Node::Ast_Node(Nonterminal_Node_Type type) {
    this->is_terminal = false;
    this->nonterminal.nonterminal_type = type;
    this->exp_info = { false, nullptr, Basic_Value() };
}
Ast_Node::Ast_Node(Terminal_Node_Type type, Basic_Value val) {
    this->is_terminal = true;
    this->terminal.terminal_type = type;
    this->terminal.basic_value = val;
    this->exp_info = { false, nullptr, Basic_Value() };
}
Ast_Node::Ast_Node(Terminal_Node_Type type, const std::string &id_name) {
    this->is_terminal = true;
    this->terminal.terminal_type = type;
    this->terminal.id_name = id_name;
    this->exp_info = { false, nullptr, Basic_Value() };
}
Ast_Node::Ast_Node(Terminal_Node_Type type, const char *id_name) {
    this->is_terminal = true;
    this->terminal.terminal_type = type;
    this->terminal.id_name = id_name;
    this->exp_info = { false, nullptr, Basic_Value() };
}
Ast_Node::Ast_Node(Nonterminal_Node_Type type, std::initializer_list<Ast_Node *> ast_nodes) {
    this->is_terminal = false;
    this->nonterminal.nonterminal_type = type;
    for (auto child : ast_nodes)
        this->nonterminal.childs.push_back(child);
    this->exp_info = { false, nullptr, Basic_Value() };
}
void Ast_Node::set_nontermianl_type(Nonterminal_Node_Type type) {
    this->is_terminal = false;
    this->nonterminal.nonterminal_type = type;
}
void Ast_Node::set_terminal_type(Terminal_Node_Type type, Basic_Value val) {
    this->is_terminal = true;
    this->terminal.terminal_type = type;
    this->terminal.basic_value = val;
}
bool Ast_Node::get_is_terminal() {
    return this->is_terminal;
}
Nonterminal_Node_Type Ast_Node::get_nonterminal_type() {
    assert(!this->is_terminal);
    return this->nonterminal.nonterminal_type;
}
Terminal_Node_Type Ast_Node::get_terminal_type() {
    assert(this->is_terminal);
    return this->terminal.terminal_type;
}
std::string Ast_Node::get_id_name() {
    assert(this->is_terminal);
    assert(this->terminal.terminal_type == ID_);
    return this->terminal.id_name;
}
Basic_Value &Ast_Node::get_basic_value() {
    assert(this->is_terminal);
    return this->terminal.basic_value;
}
std::vector<Ast_Node *> &Ast_Node::get_childs() {
    assert(!this->is_terminal);
    return this->nonterminal.childs;
}
Ast_Node::~Ast_Node() {
    if (!this->is_terminal) {
        for (auto iter = this->nonterminal.childs.begin(); iter != this->nonterminal.childs.end(); iter++) {
            delete *iter;
        }
    }
}

void Ast_Node::print() {
    if (this->is_terminal) {
        std::cout << "terminal: ";
        switch (this->terminal.terminal_type) {
        case INT_:
            std::cout << "int";
            break;
        case FLOAT_:
            std::cout << "float";
            break;
        case VOID_:
            std::cout << "void";
            break;
        case RETURN_:
            std::cout << "return";
            break;
        case IF_:
            std::cout << "if";
            break;
        case ELSE_:
            std::cout << "else";
            break;
        case WHILE_:
            std::cout << "while";
            break;
        case ID_:
            std::cout << "id: " << this->terminal.id_name;
            break;
        case SPACE_:
            std::cout << " ";
            break;
        case SEMI_:
            std::cout << ";";
            break;
        case COMMA_:
            std::cout << ",";
            break;
        case ASSIGN_:
            std::cout << "=";
            break;
            //    case RELOP_:
            //        std::cout << "void ";
            //        break;
        case L_:
            std::cout << "<";
            break;
        case G_:
            std::cout << ">";
            break;
        case LE_:
            std::cout << "<=";
            break;
        case GE_:
            std::cout << ">=";
            break;
        case EQ_:
            std::cout << "==";
            break;
        case NEQ_:
            std::cout << "!=";
            break;
        case AND_:
            std::cout << "&&";
            break;
        case OR_:
            std::cout << "||";
            break;
        case PLUS_:
            std::cout << "+";
            break;
        case MINUS_:
            std::cout << "-";
            break;
            break;
        case ADD_:
            std::cout << "+";
            break;
        case SUB_:
            std::cout << "-";
            break;
        case MUL_:
            std::cout << "*";
            break;
        case DIV_:
            std::cout << "/";
            break;
        case MOD_:
            std::cout << "%";
            break;
        case NOT_:
            std::cout << "!";
            break;
        case LP_:
            std::cout << "(";
            break;
        case RP_:
            std::cout << ")";
            break;
        case LB_:
            std::cout << "{";
            break;
        case RB_:
            std::cout << "}";
            break;
        case LC_:
            std::cout << "[";
            break;
        case RC_:
            std::cout << "]";
            break;
            //  AERROR_,
        case DOT_:
            std::cout << ",";
            break;
        case CONST_:
            std::cout << "const";
            break;
        case FOR_:
            std::cout << "for";
            break;
            //  STRING_,
        case BREAK_:
            std::cout << "break";
            break;
        case CONTINUE_:
            std::cout << "continue";
            break;
            //    PUSHZONE_,
            //    POPZONE_,
        case I32CONST_:
        case F32CONST_:
            std::cout << "num: " << this->terminal.basic_value;
            break;
        case STRING_:
            //std::cout << "num: " << this->terminal.int_num;
            break;
        case EMPTY_:
            std::cout << "empty";
            break;
        }
    }
    else {
        switch (this->nonterminal.nonterminal_type) {
        case Cond:
            std::cout << "Cond ";
            break;
        case LOrExp:
            std::cout << "LOrExp ";
            break;
        case LAndExp:
            std::cout << "LAndExp ";
            break;
        case EqExp:
            std::cout << "EqExp ";
            break;
        case RelExp:
            std::cout << "RelExp ";
            break;
        case Type_:
            std::cout << "Type ";
            break;
        case Exp:
            std::cout << "Exp ";
            break;
        case ConstExp:
            std::cout << "ConstExp ";
            break;
        case AddExp:
            std::cout << "AddExp ";
            break;
        case MulExp:
            std::cout << "MulExp ";
            break;
        case UnaryExp:
            std::cout << "UnaryExp ";
            break;
        case PrimaryExp:
            std::cout << "PrimaryExp ";
            break;
        case Call:
            std::cout << "Call ";
            break;
        case Val:
            std::cout << "Val ";
            break;
        case Str:
            std::cout << "Str ";
            break;
        case StmtExp:
            std::cout << "StmtExp ";
            break;
        case Stmt:
            std::cout << "Stmt ";
            break;
        case BlockItems:
            std::cout << "BlockItems ";
            break;
        case Block:
            std::cout << "Block ";
            break;
        case FuncRParamList:
            std::cout << "FuncRParamList ";
            break;
        case FuncRParams:
            std::cout << "FuncRParams ";
            break;
        case ArraryParameter:
            std::cout << "ArraryParameter ";
            break;
        case Declarator:
            std::cout << "Declarator ";
            break;
        case VarInitDeclarator:
            std::cout << "VarInitDeclarator ";
            break;
        case VarInitDeclaratorList:
            std::cout << "VarInitDeclaratorList ";
            break;
        case ConstInitDeclarator:
            std::cout << "ConstInitDeclarator ";
            break;
        case ConstInitDeclaratorList:
            std::cout << "ConstInitDeclaratorList ";
            break;
        case VarInitializer:
            std::cout << "VarInitializer ";
            break;
        case VarInitializerList:
            std::cout << "VarInitializerList ";
            break;
        case ConstInitializer:
            std::cout << "ConstInitializer ";
            break;
        case ConstInitializerList:
            std::cout << "ConstInitializerList ";
            break;
        case CompUnit:
            std::cout << "CompUnit ";
            break;
        case Declaration:
            std::cout << "Declaration ";
            break;
        case FuncDeclaration:
            std::cout << "FuncDeclaration ";
            break;
        case ConstDeclaration:
            std::cout << "ConstDeclaration ";
            break;
        case VarDeclaration:
            std::cout << "VarDeclaration ";
            break;
        case ParameterDeclaration:
            std::cout << "ParameterDeclaration ";
            break;
        case ParameterList:
            std::cout << "ParameterList ";
            break;
        case FuncHead:
            std::cout << "FuncHead ";
            break;
        case Parameters:
            std::cout << "Parameters ";
            break;
        }
    }
}
void Ast_Node::Traverse(int depth) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    if (this->is_terminal) {
        this->print();
        std::cout << std::endl;
    }
    else {
        this->print();
        std::cout << std::endl;
        for (auto child : this->nonterminal.childs) {
            child->Traverse(depth + 2);
        }
    }
}