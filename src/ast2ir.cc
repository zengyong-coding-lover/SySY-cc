
#include <ast_tree.hh>
#include <cassert>
#include <func.hh>
#include <graph.hh>
#include <ir.hh>
#include <program.hh>
#include <symbol.hh>
static Func *current_func;
static Basic_Block_Node *current_bb;
static Basic_Block_Node *ret_bb;
static IR_VReg *ret_addr;

static inline void current_bb_add_instr(IR_Instr *p_instr) {
    current_bb->get_info().add_instr_tail(p_instr);
    p_instr->set_block(current_bb);
}
static inline IR_VReg *current_func_new_vreg(Type *type) {
    IR_VReg *p_vreg = new IR_VReg(type);
    current_func->add_local_vreg(p_vreg);
    return p_vreg;
}
static inline void current_func_add_bb_tail(Basic_Block_Node *bb) {
    current_func->add_basic_block_tail(bb);
    bb->get_info().set_func(current_func);
}
static inline IR_VReg *Ast2IR_Assign(Basic_Value &val) {
    IR_VReg *vreg = current_func_new_vreg(val.get_type()->copy());
    IR_Instr *assign = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, vreg, new IR_Operation(val)));
    current_bb_add_instr(assign);
    return vreg;
}

static IR_VReg *Ast2IR_Val(Ast_Node *val);
static IR_VReg *Ast2IR_PrimaryExp(Ast_Node *primaryexp);
static IR_VReg *Ast2IR_UnaryExp(Ast_Node *unaryexp);
static IR_VReg *Ast2IR_MulExp(Ast_Node *mulexp);
static IR_VReg *Ast2IR_AddExp(Ast_Node *addexp);
static IR_VReg *Ast2IR_Exp(Ast_Node *exp);
static void Ast2IR_VarDeclaration(Ast_Node *vardeclaration);
static void Ast2IR_Declaration(Ast_Node *declaration);
static void Ast2IR_Block(Ast_Node *block, Basic_Block_Node *start_block, Basic_Block_Node *exit_block);

/*
FuncRParamList : FuncRParamList DOT Exp
               | Exp
*/
static void Ast2IR_FuncRParamsList(Ast_Node *funcrparamslist, IR_Instr *call) {
    assert(funcrparamslist->get_nonterminal_type() == FuncRParamList);
    if (funcrparamslist->get_childs().size() == 1) {
        IR_VReg *arg = Ast2IR_Exp(funcrparamslist->get_childs()[0]);
        call->call_add_arg(new IR_Operation(arg));
        return;
    }
    Ast2IR_FuncRParamsList(funcrparamslist->get_childs()[0], call);
    IR_VReg *arg = Ast2IR_Exp(funcrparamslist->get_childs()[2]);
    call->call_add_arg(new IR_Operation(arg));
}

/*
Call : ID LP FuncRParams RP 
FuncRParams : FuncRParamList 
            | empty 
*/
static IR_VReg *Ast2IR_Call(Ast_Node *call) {
    assert(call->get_nonterminal_type() == Call);
    Func *func = call->get_childs()[0]->exp_info.func;
    IR_VReg *dest = nullptr;
    if (!func->get_is_void())
        dest = current_func_new_vreg(func->get_ret_type()->copy());
    IR_Instr *call_instr = new IR_Instr(IR_Call_Instr(func, dest));
    Ast_Node *funcrparams = call->get_childs()[2];
    Ast_Node *funcrparamslist = funcrparams->get_childs()[0];
    if (funcrparamslist->get_is_terminal()) {
        current_bb_add_instr(call_instr);
        func->add_call_instr(call_instr);
        return dest;
    }
    Ast2IR_FuncRParamsList(funcrparamslist, call_instr);
    current_bb_add_instr(call_instr);
    func->add_call_instr(call_instr);
    return dest;
}
/*
Val : ID                
    | Val LC Exp RC   
*/

static IR_VReg *Ast2IR_Val(Ast_Node *val) {
    assert(val->get_nonterminal_type() == Val);
    if (val->get_childs().size() == 1) {
        Symbol *sym = val->get_childs()[0]->exp_info.symbol;
        if (sym->get_global()) {
            IR_VReg *dest = current_func_new_vreg(sym->get_type()->copy());
            IR_Instr *new_assign = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, dest, new IR_Operation(sym)));
            current_bb_add_instr(new_assign);
            return dest;
        }
        else
            return val->get_childs()[0]->exp_info.symbol->get_addr();
    }
    IR_VReg *addr = Ast2IR_Val(val->get_childs()[0]);
    assert(addr);
    if (addr->get_type()->get_ptr_level() > 1) {
        IR_VReg *addr1 = current_func_new_vreg(addr->get_type()->copy());
        addr1->get_type()->ptr_level_dec();
        IR_Instr *new_load = new IR_Instr(IR_Load_Instr(addr1, new IR_Operation(addr)));
        current_bb_add_instr(new_load);
        IR_VReg *dest = current_func_new_vreg(addr1->get_type()->copy());
        IR_VReg *offset = Ast2IR_Exp(val->get_childs()[2]);
        IR_Instr *new_gep = new IR_Instr(IR_Gep_Instr(dest, new IR_Operation(addr1), new IR_Operation(offset)));
        current_bb_add_instr(new_gep);
        return dest;
    }
    IR_VReg *dest = current_func_new_vreg(addr->get_type()->get_next_level()->copy());
    dest->get_type()->set_ptr_level(1); //只能为1
    IR_VReg *offset = Ast2IR_Exp(val->get_childs()[2]);
    IR_Instr *new_gep = new IR_Instr(IR_Gep_Instr(dest, new IR_Operation(addr), new IR_Operation(Basic_Value(I32CONST_t(0))), new IR_Operation(offset)));
    current_bb_add_instr(new_gep);
    return dest;
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
static IR_VReg *Ast2IR_PrimaryExp(Ast_Node *primaryexp) {
    assert(primaryexp->get_nonterminal_type() == PrimaryExp);
    if (primaryexp->get_childs().size() == 1) {
        if (primaryexp->get_childs()[0]->get_is_terminal()) {
            Basic_Value basic_value = primaryexp->get_childs()[0]->get_basic_value();
            IR_VReg *dest = current_func_new_vreg(basic_value.get_type()->copy());
            IR_Instr *new_assign = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, dest, new IR_Operation(basic_value)));
            current_bb_add_instr(new_assign);
            return dest;
        }
        switch (primaryexp->get_childs()[0]->get_nonterminal_type()) {
        case Val: {
            IR_VReg *addr = Ast2IR_Val(primaryexp->get_childs()[0]);
            if (!addr->get_type()->get_is_basic()) { // 未索引到元素级别返回下一级地址(传参用)
                IR_VReg *new_addr = current_func_new_vreg(addr->get_type()->get_next_level()->copy());
                new_addr->get_type()->ptr_level_inc();
                IR_Instr *new_gep = new IR_Instr(IR_Gep_Instr(new_addr, new IR_Operation(addr),
                    new IR_Operation(Basic_Value(I32CONST_t(0))), new IR_Operation(Basic_Value(I32CONST_t(0)))));
                current_bb_add_instr(new_gep);
                return new_addr;
            }
            IR_VReg *dest = current_func_new_vreg(addr->get_type()->copy());
            dest->get_type()->ptr_level_dec();
            IR_Instr *new_load = new IR_Instr(IR_Load_Instr(dest, new IR_Operation(addr)));
            current_bb_add_instr(new_load);
            return dest;
        }
        case Call:
            return Ast2IR_Call(primaryexp->get_childs()[0]);
        default:
            assert(0);
        }
    }
    return Ast2IR_Exp(primaryexp->get_childs()[1]);
}

/*
UnaryExp : SUB UnaryExp 
         | ADD UnaryExp     
         | NOT UnaryExp    
         | PrimaryExp       
*/
static IR_VReg *Ast2IR_UnaryExp(Ast_Node *unaryexp) {
    assert(unaryexp->get_nonterminal_type() == UnaryExp);
    if (unaryexp->get_childs().size() == 1)
        return Ast2IR_PrimaryExp(unaryexp->get_childs()[0]);
    IR_VReg *src = Ast2IR_UnaryExp(unaryexp->get_childs()[1]);
    IR_VReg *dest = current_func_new_vreg(src->get_type()->copy());
    IR_Instr *instr;
    switch (unaryexp->get_childs()[0]->get_terminal_type()) {
    case SUB_:
        instr = new IR_Instr(IR_Unary_Instr(IR_NEG, dest, new IR_Operation(src)));
        current_bb_add_instr(instr);
        break;
    case ADD_:
        instr = new IR_Instr(IR_Unary_Instr(IR_ASSIGN, dest, new IR_Operation(src)));
        current_bb_add_instr(instr);
        break;
    case NOT_:
        instr = new IR_Instr(IR_Unary_Instr(IR_NOT, dest, new IR_Operation(src)));
        current_bb_add_instr(instr);
        break;
    default:
        assert(0);
    }
    return dest;
}
/*
MulExp : MulExp MUL UnaryExp
| MulExp DIV UnaryExp
| MulExp MOD UnaryExp
| UnaryExp
*/
static IR_VReg *Ast2IR_MulExp(Ast_Node *mulexp) {
    assert(mulexp->get_nonterminal_type() == MulExp);
    if (mulexp->get_childs().size() == 1)
        return Ast2IR_UnaryExp(mulexp->get_childs()[0]);
    IR_VReg *src1 = Ast2IR_MulExp(mulexp->get_childs()[0]);
    Terminal_Node_Type bin_op = mulexp->get_childs()[1]->get_terminal_type();
    IR_VReg *src2 = Ast2IR_UnaryExp(mulexp->get_childs()[2]);

    IR_VReg *dest = current_func_new_vreg(src1->get_type()->copy());
    switch (bin_op) {
    case MUL_:
        current_bb_add_instr(new IR_Instr(IR_Binary_Instr(IR_MUL, dest, new IR_Operation(src1), new IR_Operation(src2))));
        break;
    case DIV_:
        current_bb_add_instr(new IR_Instr(IR_Binary_Instr(IR_DIV, dest, new IR_Operation(src1), new IR_Operation(src2))));
        break;
    case MOD_:
        current_bb_add_instr(new IR_Instr(IR_Binary_Instr(IR_MOD, dest, new IR_Operation(src1), new IR_Operation(src2))));
        break;
    default:
        assert(0);
    }
    return dest;
}
/*
AddExp : AddExp ADD MulExp 
       | AddExp SUB MulExp 
       | MulExp 
       ;
*/
static IR_VReg *Ast2IR_AddExp(Ast_Node *addexp) {
    assert(addexp->get_nonterminal_type() == AddExp);
    if (addexp->get_childs().size() == 1)
        return Ast2IR_MulExp(addexp->get_childs()[0]);
    IR_VReg *src1 = Ast2IR_AddExp(addexp->get_childs()[0]);
    Terminal_Node_Type bin_op = addexp->get_childs()[1]->get_terminal_type();
    IR_VReg *src2 = Ast2IR_MulExp(addexp->get_childs()[2]);

    IR_VReg *dest = current_func_new_vreg(src1->get_type()->copy());
    IR_Instr *instr;
    switch (bin_op) {
    case ADD_:
        instr = new IR_Instr(IR_Binary_Instr(IR_ADD, dest, new IR_Operation(src1), new IR_Operation(src2)));
        current_bb_add_instr(instr);
        break;
    case SUB_:
        instr = new IR_Instr(IR_Binary_Instr(IR_SUB, dest, new IR_Operation(src1), new IR_Operation(src2)));
        current_bb_add_instr(instr);
        break;
    default:
        assert(0);
    }
    return dest;
}

static IR_VReg *Ast2IR_Exp(Ast_Node *exp) {
    assert(exp->get_nonterminal_type() == Exp);
    if (exp->exp_info.is_const)
        return Ast2IR_Assign(exp->exp_info.node_val);
    return Ast2IR_AddExp(exp->get_childs()[0]);
}

/*
VarDeclaration : VarInitDeclaratorList SEMI
VarInitDeclaratorList : VarInitDeclaratorList DOT VarInitDeclarator
                      | Type VarInitDeclarator
*/
static inline void new_initials(Initializer *initials, IR_VReg *addr) {
    if (!initials->get_is_list()) {
        IR_Operation *src;
        if (!initials->get_is_val()) {
            IR_VReg *val = Ast2IR_Exp(initials->get_init_exp());
            src = new IR_Operation(val);
        }
        else
            src = new IR_Operation(initials->get_init_val());
        IR_Instr *store = new IR_Instr(IR_Store_Instr(src, new IR_Operation(addr)));
        current_bb_add_instr(store);
        return;
    }
    size_t i = 0;
    for (auto initial : initials->get_initials()) {
        IR_VReg *dest = current_func_new_vreg(addr->get_type()->get_next_level()->copy());
        dest->get_type()->ptr_level_inc();
        IR_Instr *gep_instr = new IR_Instr(IR_Gep_Instr(dest, new IR_Operation(addr),
            new IR_Operation(Basic_Value((I32CONST_t) 0)), new IR_Operation((I32CONST_t) i++)));
        current_bb_add_instr(gep_instr);
        new_initials(initial, dest);
    }
}
static inline void new_alloca_sym(Symbol *sym) {
    assert(sym);
    IR_VReg *dest = current_func_new_vreg(sym->get_type()->copy());
    dest->get_type()->ptr_level_inc();
    IR_Instr *alloca = new IR_Instr(IR_Alloca_Instr(dest, sym));
    current_bb_add_instr(alloca);
    Initializer *initials = sym->get_initials();
    if (initials)
        new_initials(initials, dest);
    sym->set_addr(dest);
}
/*
VarDeclaration : VarInitDeclaratorList SEMI
*/
/*
VarInitDeclaratorList : VarInitDeclaratorList DOT VarInitDeclarator 
                      | Type VarInitDeclarator
*/
static void Ast2IR_VarDeclaration(Ast_Node *vardeclaration) {
    assert(vardeclaration->get_nonterminal_type() == VarDeclaration);
    Ast_Node *varinitdeclaratorlist = vardeclaration->get_childs()[0];
    while (varinitdeclaratorlist->get_childs().size() == 3) {
        Symbol *sym = varinitdeclaratorlist->get_childs()[2]->exp_info.symbol;
        new_alloca_sym(sym);
        varinitdeclaratorlist = varinitdeclaratorlist->get_childs()[0];
    }
    Symbol *sym = varinitdeclaratorlist->get_childs()[1]->exp_info.symbol;
    new_alloca_sym(sym);
}
/*
Declaration : ConstDeclaration 
            | VarDeclaration 
*/
static void Ast2IR_Declaration(Ast_Node *declaration) {
    assert(declaration->get_nonterminal_type() == Declaration);
    Ast_Node *child = declaration->get_childs()[0];
    switch (child->get_nonterminal_type()) {
    case ConstDeclaration: // const 类型不用生成 ir
        break;
    case VarDeclaration:
        Ast2IR_VarDeclaration(child);
        break;
    default:
        assert(0);
    }
}
/*
RelExp : RelExp L AddExp 
       | RelExp G AddExp 
       | RelExp LE AddExp 
       | RelExp GE AddExp 
       | AddExp 
*/
static IR_VReg *Ast2IR_RelExp(Ast_Node *relexp) {
    assert(relexp->get_nonterminal_type() == RelExp);
    if (relexp->get_childs().size() == 1) {
        return Ast2IR_AddExp(relexp->get_childs()[0]);
    }
    IR_VReg *src1 = Ast2IR_RelExp(relexp->get_childs()[0]);
    IR_VReg *src2 = Ast2IR_AddExp(relexp->get_childs()[2]);
    IR_VReg *des = current_func_new_vreg(new Type(I32));
    IR_Instr *new_instr;
    switch (relexp->get_childs()[1]->get_terminal_type()) {
    case L_:
        new_instr = new IR_Instr(IR_Binary_Instr(IR_LT, des, new IR_Operation(src1), new IR_Operation(src2)));
        break;
    case G_:
        new_instr = new IR_Instr(IR_Binary_Instr(IR_GT, des, new IR_Operation(src1), new IR_Operation(src2)));
        break;
    case LE_:
        new_instr = new IR_Instr(IR_Binary_Instr(IR_LE, des, new IR_Operation(src1), new IR_Operation(src2)));
        break;
    case GE_:
        new_instr = new IR_Instr(IR_Binary_Instr(IR_GE, des, new IR_Operation(src1), new IR_Operation(src2)));
        break;
    default:
        assert(0);
    }
    current_bb_add_instr(new_instr);
    return des;
}
/*
EqExp : EqExp EQ RelExp 
      | EqExp NEQ RelExp
      | RelExp 
*/
static IR_VReg *Ast2IR_EqExp(Ast_Node *eqexp) {
    assert(eqexp->get_nonterminal_type() == EqExp);
    if (eqexp->get_childs().size() == 1) {
        return Ast2IR_RelExp(eqexp->get_childs()[0]);
    }
    IR_VReg *src1 = Ast2IR_EqExp(eqexp->get_childs()[0]);
    IR_VReg *src2 = Ast2IR_RelExp(eqexp->get_childs()[2]);
    IR_VReg *des = current_func_new_vreg(new Type(I32));
    IR_Instr *new_instr;
    switch (eqexp->get_childs()[1]->get_terminal_type()) {
    case EQ_:
        new_instr = new IR_Instr(IR_Binary_Instr(IR_EQ, des, new IR_Operation(src1), new IR_Operation(src2)));
        break;
    case NEQ_:
        new_instr = new IR_Instr(IR_Binary_Instr(IR_NE, des, new IR_Operation(src1), new IR_Operation(src2)));
        break;
    default:
        assert(0);
    }
    current_bb_add_instr(new_instr);
    return des;
}
/*
LAndExp : LAndExp AND EqExp
        | EqExp
*/
static void Ast2IR_LAndExp(Ast_Node *landexp, Basic_Block_Node *true_block, Basic_Block_Node *false_block) {
    assert(landexp->get_nonterminal_type() == LAndExp);
    if (landexp->get_childs().size() == 1) {
        IR_VReg *cond = Ast2IR_EqExp(landexp->get_childs()[0]);
        IR_Instr *new_bl = new IR_Instr(IR_Br_Instr(new IR_Operation(cond), new Basic_Block_Edge(current_bb, true_block), new Basic_Block_Edge(current_bb, false_block)));
        current_bb_add_instr(new_bl);
        return;
    }
    Basic_Block_Node *new_block = new Basic_Block_Node();
    current_func_add_bb_tail(new_block);
    Ast2IR_LAndExp(landexp->get_childs()[0], new_block, false_block);
    current_bb = new_block;
    IR_VReg *cond = Ast2IR_EqExp(landexp->get_childs()[2]);
    IR_Instr *new_bl = new IR_Instr(IR_Br_Instr(new IR_Operation(cond), new Basic_Block_Edge(current_bb, true_block), new Basic_Block_Edge(current_bb, false_block)));
    current_bb_add_instr(new_bl);
}
/*
LOrExp : LOrExp OR LAndExp
       | LAndExp
*/
static void Ast2IR_LOrExp(Ast_Node *lorexp, Basic_Block_Node *true_block, Basic_Block_Node *false_block) {
    assert(lorexp->get_nonterminal_type() == LOrExp);
    // 完成：参考 and 的短路求值（Ast2IR_LAndExp）， 实现 or 的短路求值
    if (lorexp->get_childs().size() == 1) {
        Ast2IR_LAndExp(lorexp->get_childs()[0], true_block, false_block);
        return;
    }
    Basic_Block_Node *and_block = new Basic_Block_Node();
    current_func_add_bb_tail(and_block);
    Ast2IR_LOrExp(lorexp->get_childs()[0], true_block, and_block);
    current_bb = and_block;
    Ast2IR_LAndExp(lorexp->get_childs()[2], true_block, false_block);
}
/*
Cond : LOrExp
*/
static void Ast2IR_Cond(Ast_Node *cond, Basic_Block_Node *true_block, Basic_Block_Node *false_block) {
    assert(cond->get_nonterminal_type() == Cond);
    Ast2IR_LOrExp(cond->get_childs()[0], true_block, false_block);
}
/*
StmtExp : empty 
| Exp 
*/
static IR_VReg *Ast2IR_StmtExp(Ast_Node *stmt_exp) {
    assert(stmt_exp->get_nonterminal_type() == StmtExp);
    if (stmt_exp->exp_info.is_const)
        return Ast2IR_Assign(stmt_exp->exp_info.node_val);
    if (!stmt_exp->get_childs()[0]->get_is_terminal())
        return Ast2IR_Exp(stmt_exp->get_childs()[0]);
    return nullptr;
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
static void Ast2IR_Stmt(Ast_Node *stmt, Basic_Block_Node *cond_block, Basic_Block_Node *exit_block) {
    assert(stmt->get_nonterminal_type() == Stmt);
    Ast_Node *child = stmt->get_childs()[0];
    IR_Instr *new_instr = nullptr;
    IR_Instr *new_bl = nullptr;
    IR_VReg *vreg1 = nullptr;
    if (child->get_is_terminal()) {
        switch (child->get_terminal_type()) {
        case RETURN_:
            vreg1 = Ast2IR_StmtExp(stmt->get_childs()[1]);
            if (vreg1) { // 返回类型不是 void
                new_instr = new IR_Instr(IR_Store_Instr(new IR_Operation(vreg1), new IR_Operation(ret_addr)));
                current_bb_add_instr(new_instr);
            }
            new_bl = new IR_Instr(IR_Br_Instr(new Basic_Block_Edge(current_bb, ret_bb)));
            current_bb_add_instr(new_bl);
            current_bb = new Basic_Block_Node();
            current_func_add_bb_tail(current_bb);
            break;
        case BREAK_:
            new_bl = new IR_Instr(IR_Br_Instr(new Basic_Block_Edge(current_bb, exit_block)));
            current_bb_add_instr(new_bl);
            current_bb = new Basic_Block_Node();
            current_func_add_bb_tail(current_bb);
            break;
        case CONTINUE_:
            // 完成：添加跳转指令，跳转到该层循环的条件判断块
            new_bl = new IR_Instr(IR_Br_Instr(new Basic_Block_Edge(current_bb, cond_block)));
            current_bb_add_instr(new_bl);
            current_bb = new Basic_Block_Node();
            current_func_add_bb_tail(current_bb);
            break;
        case IF_: {
            Basic_Block_Node *true_block = new Basic_Block_Node();
            Basic_Block_Node *next_block = new Basic_Block_Node();
            if (stmt->get_childs().size() == 7) {
                Basic_Block_Node *false_block = new Basic_Block_Node();
                Ast2IR_Cond(stmt->get_childs()[2], true_block, false_block);
                current_func_add_bb_tail(true_block);
                current_bb = true_block;
                Ast2IR_Stmt(stmt->get_childs()[4], cond_block, exit_block);
                new_bl = new IR_Instr(IR_Br_Instr(new Basic_Block_Edge(current_bb, next_block)));
                current_bb_add_instr(new_bl);
                current_bb = false_block;
                Ast2IR_Stmt(stmt->get_childs()[6], cond_block, exit_block);
                new_bl = new IR_Instr(IR_Br_Instr(new Basic_Block_Edge(current_bb, next_block)));
                current_bb_add_instr(new_bl);
                current_bb = next_block;
                current_func_add_bb_tail(false_block);
                current_func_add_bb_tail(next_block);
                break;
            }
            Ast2IR_Cond(stmt->get_childs()[2], true_block, next_block);
            current_func_add_bb_tail(true_block);
            current_bb = true_block;
            Ast2IR_Stmt(stmt->get_childs()[4], cond_block, exit_block);
            new_bl = new IR_Instr(IR_Br_Instr(new Basic_Block_Edge(current_bb, next_block)));
            current_bb_add_instr(new_bl);
            current_bb = next_block;
            current_func_add_bb_tail(next_block);
            break;
        }
        case WHILE_: {
            Basic_Block_Node *start_block = new Basic_Block_Node();
            current_func_add_bb_tail(start_block);
            Basic_Block_Node *exit_block = new Basic_Block_Node();
            Ast2IR_Cond(stmt->get_childs()[2], start_block, exit_block);
            current_bb = start_block;
            Basic_Block_Node *cond_block = new Basic_Block_Node();
            Ast2IR_Stmt(stmt->get_childs()[4], cond_block, exit_block);
            new_bl = new IR_Instr(IR_Br_Instr(new Basic_Block_Edge(current_bb, cond_block)));
            current_bb_add_instr(new_bl);
            current_bb = cond_block;
            Ast2IR_Cond(stmt->get_childs()[2], start_block, exit_block);
            current_bb = exit_block;
            current_func_add_bb_tail(cond_block);
            current_func_add_bb_tail(exit_block);
            break;
        }
        default:
            assert(0);
        }
        return;
    }
    switch (child->get_nonterminal_type()) {
    case Block:
        Ast2IR_Block(child, cond_block, exit_block);
        break;
    case Val: {
        IR_VReg *des = Ast2IR_Val(child);
        IR_VReg *src = Ast2IR_Exp(stmt->get_childs()[2]);
        IR_Instr *instr = new IR_Instr(IR_Store_Instr(new IR_Operation(src), new IR_Operation(des)));
        current_bb_add_instr(instr);
        break;
    }
    case StmtExp:
        Ast2IR_StmtExp(child);
        break;
    default:
        assert(0);
    }
}
/*
BlockItems : BlockItems Declaration 
           | BlockItems Stmt          
           | empty
*/
static void Ast2IR_BlockItems(Ast_Node *blockitems, Basic_Block_Node *cond_block, Basic_Block_Node *exit_block) {
    assert(blockitems->get_nonterminal_type() == BlockItems);
    if (blockitems->get_childs().size() == 1)
        return;
    Ast2IR_BlockItems(blockitems->get_childs()[0], cond_block, exit_block);
    Ast_Node *child2 = blockitems->get_childs()[1];
    switch (child2->get_nonterminal_type()) {
    case Declaration:
        Ast2IR_Declaration(child2);
        break;
    case Stmt:
        Ast2IR_Stmt(child2, cond_block, exit_block);
        break;
    default:
        assert(0);
    }
}
/*
Block : LB BlockItems RB
*/
static void Ast2IR_Block(Ast_Node *block, Basic_Block_Node *cond_block, Basic_Block_Node *exit_block) {
    assert(block->get_nonterminal_type() == Block);
    Ast_Node *blockitems = block->get_childs()[1];
    Ast2IR_BlockItems(blockitems, cond_block, exit_block);
}
static void add_ret(Func *func) {
    Ast_Node *stmtexp = nullptr;
    if (!func->get_is_void()) {
        Ast_Node *ret_val = nullptr;
        switch (func->get_ret_type()->get_basic_type()) {
        case I32:
            ret_val = new Ast_Node(I32CONST_, Basic_Value(I32CONST_t(0)));
            break;
        case F32:
            ret_val = new Ast_Node(F32CONST_, Basic_Value(F32CONST_t(0)));
            break;
        }
        Ast_Node *primaryexp = new Ast_Node(PrimaryExp, { ret_val });
        Ast_Node *unaryexp = new Ast_Node(UnaryExp, { primaryexp });
        Ast_Node *mulexp = new Ast_Node(MulExp, { unaryexp });
        Ast_Node *addexp = new Ast_Node(AddExp, { mulexp });
        Ast_Node *exp = new Ast_Node(Exp, { addexp });
        stmtexp = new Ast_Node(StmtExp, { exp });
    }
    else {
        stmtexp = new Ast_Node(StmtExp, { new Ast_Node(EMPTY_) });
    }
    Ast_Node *stmt = new Ast_Node(Stmt, { new Ast_Node(RETURN_), stmtexp, new Ast_Node(SEMI_) });
    Ast_Node *block = func->get_ast_body();

    Ast_Node *blockitems = new Ast_Node(BlockItems, { block->get_childs()[1], stmt });
    block->get_childs()[1] = blockitems;
}
void Ast2IR_Func(Func *func) {
    if (func->get_is_lib()) return;
    add_ret(func);
    current_func = func;
    Ast_Node *block = func->get_ast_body();
    ret_bb = new Basic_Block_Node();
    Basic_Block_Node *begin_bb = new Basic_Block_Node();
    current_func_add_bb_tail(begin_bb);
    func->set_entry_bb(begin_bb);
    current_bb = begin_bb;
    // 处理函数参数
    for (auto &sym : func->get_params()) {
        IR_VReg *reg_param;
        if (sym->get_type()->get_is_basic())
            reg_param = new IR_VReg(sym->get_type()->copy());
        else {
            Type *type = sym->get_type()->copy();
            reg_param = new IR_VReg(type);
        }
        func->add_param_vreg(reg_param);
        IR_VReg *addr = current_func_new_vreg(reg_param->get_type()->copy());
        addr->get_type()->ptr_level_inc();
        IR_Instr *new_alloca = new IR_Instr(IR_Alloca_Instr(addr, sym));
        current_bb_add_instr(new_alloca);
        IR_Instr *new_store = new IR_Instr(IR_Store_Instr(new IR_Operation(reg_param), new IR_Operation(addr)));
        current_bb_add_instr(new_store);
        sym->set_addr(addr);
    }
    // 处理返回值
    if (!func->get_is_void()) {
        Symbol *ret_sym = new Symbol("--" + func->get_name() + "_ret--", func->get_ret_type()->copy(), false, false, 0, false, 0, func, nullptr);
        ret_addr = current_func_new_vreg(func->get_ret_type()->copy());
        ret_addr->get_type()->ptr_level_inc();
        IR_Instr *new_alloc = new IR_Instr(IR_Alloca_Instr(ret_addr, ret_sym));
        begin_bb->get_info().add_instr_tail(new_alloc);
        IR_VReg *ret_val = current_func_new_vreg(func->get_ret_type()->copy());
        IR_Instr *new_load = new IR_Instr(IR_Load_Instr(ret_val, new IR_Operation(ret_addr)));
        IR_Instr *new_ret = new IR_Instr(IR_Ret_Instr(new IR_Operation(ret_val)));
        ret_bb->get_info().add_instr_tail(new_load);
        ret_bb->get_info().add_instr_tail(new_ret);
    }
    else {
        IR_Instr *new_ret = new IR_Instr(IR_Ret_Instr(nullptr));
        ret_bb->get_info().add_instr_tail(new_ret);
    }
    Ast2IR_Block(block, nullptr, nullptr);
    current_func_add_bb_tail(ret_bb);
    func->set_exit_bb(ret_bb);
}
void Ast2IR_Program(Program *program) {
    assert(program->get_lang_level() == AST);
    for (auto func : program->get_funcs()) {
        Ast2IR_Func(func);
    }
    program->set_lang_level(IR);
}