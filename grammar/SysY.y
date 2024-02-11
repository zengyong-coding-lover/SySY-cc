/*
 * SysY.y : Parser for SysY language
 */
%define parse.error verbose
%define api.pure full
%parse-param {Program *program}

%{
// for parser
%}

%code requires{
#include <ast_tree.hh>
#include <program.hh>
}

%union {
// AST struct
 Ast_Node *ast_node;
}

%define api.token.prefix {TOK_}

// %type <type> Type

%token <ast_node> INT FLOAT RETURN IF ELSE WHILE ID SPACE SEMI COMMA ASSIGN RELOP PLUS
MINUS DIV AND OR DOT NOT LP RP LB RB LC RC AERROR CONST VOID MOD  LE GE EQ NEQ
 FOR STRING PUSHZONE POPZONE  I32CONST F32CONST L G ADD SUB MUL 
%token <ast_node> BREAK CONTINUE
%token <ast_node> EOL

%type <ast_node> Cond
%type <ast_node> LOrExp
%type <ast_node> LAndExp
%type <ast_node> EqExp
%type <ast_node> RelExp
%type <ast_node> Type

%type <ast_node> Exp
%type <ast_node> ConstExp
%type <ast_node> AddExp
%type <ast_node> MulExp
%type <ast_node> UnaryExp
%type <ast_node> PrimaryExp

%type <ast_node> Call
%type <ast_node> Val
%type <ast_node> Str

%type <ast_node> StmtExp
%type <ast_node> Stmt

%type <ast_node> BlockItems
%type <ast_node> Block

%type <ast_node> FuncRParamList
%type <ast_node> FuncRParams

%type <ast_node> ArraryParameter

%type <ast_node> Declarator
%type <ast_node> VarInitDeclarator
%type <ast_node> VarInitDeclaratorList
%type <ast_node> ConstInitDeclarator
%type <ast_node> ConstInitDeclaratorList

%type <ast_node> VarInitializer
%type <ast_node> VarInitializerList
%type <ast_node> ConstInitializer
%type <ast_node> ConstInitializerList

%type <ast_node> CompUnit Declaration FuncDeclaration ConstDeclaration VarDeclaration 
%type <ast_node> ParameterDeclaration ParameterList FuncHead Parameters 

%nonassoc NO_ELSE
%nonassoc ELSE

%code {
#include <SysY.yy.h>
void yyerror(Program *program, const char *str) { }
}
%%
S : CompUnit { TODO();/*完成：设置 program 的 ast_root 为 CompUnit指向的节点*/ }

CompUnit : CompUnit Declaration { $$ = new Ast_Node(CompUnit, {$1, $2});}
         | CompUnit FuncDeclaration { $$ = new Ast_Node(CompUnit, {$1, $2});}
         | /* *empty */             { TODO();/*完成：新建空子节点*/ }
         ;

Type : INT   {  $$ = new Ast_Node(Type_, {$1});}
     | FLOAT {  $$ = new Ast_Node(Type_, {$1});}
     ;

Declaration : ConstDeclaration {$$ = new Ast_Node(Declaration, {$1});}
            | VarDeclaration {$$ = new Ast_Node(Declaration, {$1});}
            ;

ConstDeclaration : ConstInitDeclaratorList SEMI { $$ = new Ast_Node(ConstDeclaration, {$1, $2}); }
                 ;

VarDeclaration : VarInitDeclaratorList SEMI { $$ = new Ast_Node(VarDeclaration, {$1, $2}); }
               ;

ConstInitDeclaratorList : ConstInitDeclaratorList DOT ConstInitDeclarator { $$ = new Ast_Node(ConstInitDeclaratorList, {$1, $2, $3}); }
                        | CONST Type ConstInitDeclarator                  { $$ = new Ast_Node(ConstInitDeclaratorList, {$1, $2, $3}); }
                        ;

VarInitDeclaratorList : VarInitDeclaratorList DOT VarInitDeclarator { $$ = new Ast_Node(VarInitDeclaratorList, {$1, $2, $3}); }
                      | Type VarInitDeclarator                      { $$ = new Ast_Node(VarInitDeclaratorList, {$1, $2}); }
                      ;

ConstInitDeclarator : Declarator ASSIGN ConstInitializer {  $$ = new Ast_Node(ConstInitDeclarator, {$1, $2, $3});}
                    ;

VarInitDeclarator : Declarator ASSIGN VarInitializer { $$ = new Ast_Node(VarInitDeclarator, {$1, $2, $3}); }
                  | Declarator                    { $$ = new Ast_Node(VarInitDeclarator, {$1}); }
                  ;

Declarator : Declarator LC ConstExp RC { $$ = new Ast_Node(Declarator, {$1, $2, $3, $4}); }
           | ID                          { $$ = new Ast_Node(Declarator, {$1}); }
           ;

ConstInitializer : LB ConstInitializerList RB     { $$ = new Ast_Node(ConstInitializer, {$1, $2, $3}); }
                 | LB RB                          { $$ = new Ast_Node(ConstInitializer); }
                 | ConstExp                         { $$ = new Ast_Node(ConstInitializer, {$1}); }
                 ;

ConstInitializerList : ConstInitializerList DOT ConstInitializer { $$ = new Ast_Node(ConstInitializerList, {$1, $2, $3}); }
                     | ConstInitializer                          { $$ = new Ast_Node(ConstInitializerList, {$1}); }
                     ;

VarInitializer : LB VarInitializerList RB     { $$ = new Ast_Node(VarInitializer, {$1, $2, $3}); }
               | LB RB                        { $$ = new Ast_Node(VarInitializer, {$1, $2}); }
               | Exp                      { $$ = new Ast_Node(VarInitializer, {$1}); }
               ;

VarInitializerList : VarInitializerList DOT VarInitializer { $$ = new Ast_Node(VarInitializerList, {$1, $2, $3}); }
                   | VarInitializer                        { $$ = new Ast_Node(VarInitializerList, {$1}); }
                   ;

FuncHead : Type ID { $$ = new Ast_Node(FuncHead, {$1, $2}); }
         | VOID ID { $$ = new Ast_Node(FuncHead, {$1, $2}); }
         ;

FuncDeclaration : FuncHead LP Parameters RP Block { $$ = new Ast_Node(FuncDeclaration, {$1, $2, $3, $4, $5}); }
                ;

Parameters : ParameterList { $$ = new Ast_Node(Parameters, {$1}); }
           | /* *empty */ { $$ = new Ast_Node(Parameters, {new Ast_Node(EMPTY_)}); }
           ;

ParameterList : ParameterList DOT ParameterDeclaration { $$ = new Ast_Node(ParameterList, {$1, $2, $3}); }
              | ParameterDeclaration { $$ = new Ast_Node(ParameterList, {$1}); }
              ;

ParameterDeclaration : Type ArraryParameter { $$ = new Ast_Node(ParameterDeclaration, {$1, $2}); }
                     | Type ID              { $$ = new Ast_Node(ParameterDeclaration, {$1, $2}); }
                     ;

ArraryParameter : ID LC RC                  { $$ = new Ast_Node(ArraryParameter, {$1, $2, $3}); }
                | ArraryParameter LC ConstExp RC { $$ = new Ast_Node(ArraryParameter, {$1, $2, $3, $4}); }
                ;

Cond : LOrExp { $$ = new Ast_Node(Cond, {$1}); }
     ;

LOrExp : LOrExp OR LAndExp { $$ = new Ast_Node(LOrExp, {$1, $2, $3}); }
       | LAndExp { $$ = new Ast_Node(LOrExp, {$1}); }
       ;

LAndExp : LAndExp AND EqExp { $$ = new Ast_Node(LAndExp, {$1, $2, $3}); }
        | EqExp             { $$ = new Ast_Node(LAndExp,{$1}); }
        ;

EqExp : EqExp EQ RelExp  { $$ = new Ast_Node(EqExp, {$1, $2, $3}); }
      | EqExp NEQ RelExp { $$ = new Ast_Node(EqExp, {$1, $2, $3}); }
      | RelExp { $$ = new Ast_Node(EqExp, {$1}); }
      ;

RelExp : RelExp L AddExp { $$ = new Ast_Node(RelExp, {$1, $2, $3}); }
       | RelExp G AddExp { $$ = new Ast_Node(RelExp, {$1, $2, $3}); }
       | RelExp LE AddExp  { $$ = new Ast_Node(RelExp, {$1, $2, $3}); }
       | RelExp GE AddExp  { $$ = new Ast_Node(RelExp, {$1, $2,$3}); }
       | AddExp { $$ = new Ast_Node(RelExp, {$1}); }
       ;

ConstExp : Exp { $$ = new Ast_Node(ConstExp, {$1}); }
         ;

Exp : AddExp { $$ = new Ast_Node(Exp, {$1});}
    ;

AddExp : AddExp ADD MulExp { $$ = new Ast_Node(AddExp, {$1, $2, $3}); }
       | AddExp SUB MulExp { $$ = new Ast_Node(AddExp, {$1, $2, $3}); }
       | MulExp { $$ = new Ast_Node(AddExp, {$1}); }
       ;

MulExp : MulExp MUL UnaryExp { $$ = new Ast_Node(MulExp, {$1, $2, $3}); }
       | MulExp DIV UnaryExp { $$ = new Ast_Node(MulExp, {$1, $2, $3}); }
       | MulExp MOD UnaryExp { $$ = new Ast_Node(MulExp, {$1, $2, $3}); }
       | UnaryExp { $$ = new Ast_Node(MulExp, {$1}); }
       ;

UnaryExp : SUB UnaryExp     { $$ = new Ast_Node(UnaryExp, {$1, $2}); }
         | ADD UnaryExp     { $$ = new Ast_Node(UnaryExp, {$1, $2}); }
         | NOT UnaryExp     { $$ = new Ast_Node(UnaryExp, {$1, $2}); }
         | PrimaryExp       { $$ = new Ast_Node(UnaryExp, {$1}); }
         ;

PrimaryExp : LP Exp RP { $$ = new Ast_Node(PrimaryExp, {$1, $2, $3}); }
           | I32CONST    { $$ = new Ast_Node(PrimaryExp, {$1}); }
           | F32CONST    { $$ = new Ast_Node(PrimaryExp, {$1}); }
           | Val         { $$ = new Ast_Node(PrimaryExp, {$1}); }
           | Call        { $$ = new Ast_Node(PrimaryExp, {$1}); }
           | Str         { $$ = new Ast_Node(PrimaryExp, {$1}); }
           ;

Call : ID LP FuncRParams RP { $$ = new Ast_Node(Call, {$1, $2, $3, $4});  }
     ;

Val : ID                 { $$ = new Ast_Node(Val, {$1});  }
    | Val LC Exp RC    { $$ = new Ast_Node(Val, {$1, $2, $3, $4}); }
    ;

Str : STRING { $$ = new Ast_Node(Str, {$1}); }
    ;

FuncRParams : FuncRParamList { $$ = new Ast_Node(FuncRParams, {$1}); }
            | /* *empty */   { $$ = new Ast_Node(FuncRParams, {new Ast_Node(EMPTY_)}); }
            ;

FuncRParamList : FuncRParamList DOT Exp { $$ = new Ast_Node(FuncRParamList, {$1, $2, $3}); }
               | Exp                    { $$ = new Ast_Node(FuncRParamList, {$1}); }
               ;

Block : LB BlockItems RB { $$ = new Ast_Node(Block, {$1, $2, $3}); }
      ;

BlockItems : BlockItems Declaration { $$ = new Ast_Node(BlockItems, {$1, $2});}
           | BlockItems Stmt           {  $$ = new Ast_Node(BlockItems, {$1, $2}); }
           | /* *empty */              {  $$ = new Ast_Node(BlockItems, {new Ast_Node(EMPTY_)}); }
           ;

StmtExp : /* *empty */ { $$ = new Ast_Node(StmtExp, {new Ast_Node(EMPTY_)}); }
        | Exp { $$ = new Ast_Node(StmtExp, {$1});}
        ;

Stmt : Block              { $$ = new Ast_Node(Stmt, {$1}); }
     | Val ASSIGN Exp SEMI                    { $$ = new Ast_Node(Stmt, {$1, $2, $3, $4}); }
     | StmtExp SEMI                        { $$ = new Ast_Node(Stmt, {$1, $2}); }
     | RETURN StmtExp SEMI                 { $$ = new Ast_Node(Stmt, {$1, $2, $3}); }
     | BREAK SEMI                          { $$ = new Ast_Node(Stmt, {$1, $2}); }
     | CONTINUE SEMI                       { $$ = new Ast_Node(Stmt, {$1, $2}); }
     | IF LP Cond RP Stmt ELSE Stmt     { $$ = new Ast_Node(Stmt, {$1, $2, $3, $4, $5, $6, $7}); }
     | IF LP Cond RP Stmt %prec NO_ELSE { $$ = new Ast_Node(Stmt, {$1, $2, $3, $4, $5});}
     | WHILE LP Cond RP Stmt            { $$ = new Ast_Node(Stmt, {$1, $2, $3, $4, $5}); }
     ;

%%
