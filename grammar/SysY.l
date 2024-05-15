/*
 * SysY.l : Scanner for SysY language
 */
%{
#include <SysY.tab.h>

I32CONST_t str2int(const char* text, int base);
F32CONST_t str2float(const char *text);
%}

%option bison-bridge
%option yylineno batch noinput nounput noyywrap

%x MULTICOMMENT
%x LINECOMMENT

%x header

L    (<)
G    (>)
LE   (<=)
GE   (>=)
EQ   (==)
NEQ  (!=)

AND  (&&)
OR   (\|\|)
ADD  (\+)
SUB  (-)
MUL  (\*)
DIV  (\/)
MOD  (%)

INT       (int)
FLOAT     (float)
VOID      (void)
CONST     (const)
WHILE     (while)
BREAK     (break)
CONTINUE  (continue)
IF        (if)
ELSE      (else)
RETURN    (return)

BLANK     ([ \t\r\a])
NEWLINE   ([\n])

ID        ([_a-zA-Z][_a-zA-Z0-9]*)

HEXI32CONST  (0[xX][0-9a-fA-F]+)
OCTI32CONST  (0[0-7]*)
DECI32CONST  ([1-9][0-9]*)

HEXFLOATCONST  ((0[xX])(([0-9a-fA-F]*\.?[0-9a-fA-F]+)|([0-9a-fA-F]+\.))([pP][-+]?[0-9]+))
DECFLOATCONST  (([0-9]+[eE][-+]?[0-9]+)|((([0-9]+\.)|([0-9]*\.[0-9]+))([eE][-+]?[0-9]+)?))

STRING (["][^"\n]*["])

MACRO_STARTTIME "starttime"({BLANK}|{NEWLINE})*"()"
MACRO_STOPTIME  "stoptime"({BLANK}|{NEWLINE})*"()"
MACRO_LINE      "__LINE__"

%%

[!] { yylval->ast_node = new Ast_Node(NOT_);return TOK_NOT;  }

{L}  { yylval->ast_node = new Ast_Node(L_);return TOK_L;  }
{G}  { yylval->ast_node = new Ast_Node(G_);return TOK_G;  }
{LE}  { yylval->ast_node = new Ast_Node(LE_);return TOK_LE;  }
{GE}  { yylval->ast_node = new Ast_Node(GE_);return TOK_GE;  }
{EQ}  { yylval->ast_node = new Ast_Node(EQ_);return TOK_EQ;  }
{NEQ} { yylval->ast_node = new Ast_Node(NEQ_);return TOK_NEQ; }

{AND} { yylval->ast_node = new Ast_Node(AND_);return TOK_AND; }
{OR}  { yylval->ast_node = new Ast_Node(OR_);return TOK_OR;  }
{ADD}  { yylval->ast_node = new Ast_Node(ADD_);return TOK_ADD;  }
{SUB}  { yylval->ast_node = new Ast_Node(SUB_);return TOK_SUB;  }
{MUL}  { yylval->ast_node = new Ast_Node(MUL_);return TOK_MUL;  }
{DIV}  { yylval->ast_node = new Ast_Node(DIV_);return TOK_DIV;  }
{MOD}  { yylval->ast_node = new Ast_Node(MOD_);return TOK_MOD;  }

"("  { yylval->ast_node = new Ast_Node(LP_);return TOK_LP;  }
")"   { yylval->ast_node = new Ast_Node(RP_);return TOK_RP;  }
"{"  { yylval->ast_node = new Ast_Node(LB_);return TOK_LB;  }
"}"  { yylval->ast_node = new Ast_Node(RB_);return TOK_RB;  }
"["  { yylval->ast_node = new Ast_Node(LC_);return TOK_LC;  }
"]"  { yylval->ast_node = new Ast_Node(RC_);return TOK_RC;  }

","  { yylval->ast_node = new Ast_Node(DOT_);return TOK_DOT;  }
";"  { yylval->ast_node = new Ast_Node(SEMI_);return TOK_SEMI;  }

"="  { yylval->ast_node = new Ast_Node(ASSIGN_);return TOK_ASSIGN;  }

{BLANK}+   { /* empty space */ }
{NEWLINE}+ { /* empty lines */ }

{INT}      { yylval->ast_node = new Ast_Node(INT_);return TOK_INT;      }
{FLOAT}    { yylval->ast_node = new Ast_Node(FLOAT_);return TOK_FLOAT;    }
{VOID}     { yylval->ast_node = new Ast_Node(VOID_);return TOK_VOID;     }
{CONST}    { yylval->ast_node = new Ast_Node(CONST_);return TOK_CONST;    }
{WHILE}    { yylval->ast_node = new Ast_Node(WHILE_);return TOK_WHILE;    }
{BREAK}    { yylval->ast_node = new Ast_Node(BREAK_);return TOK_BREAK;    }
{CONTINUE} { yylval->ast_node = new Ast_Node(CONTINUE_);return TOK_CONTINUE; }
{IF}       { yylval->ast_node = new Ast_Node(IF_);return TOK_IF;       }
{ELSE}     { yylval->ast_node = new Ast_Node(ELSE_);return TOK_ELSE;     }
{RETURN}   { yylval->ast_node = new Ast_Node(RETURN_);return TOK_RETURN;   }

{HEXI32CONST}  { yylval->ast_node = new Ast_Node(I32CONST_, Basic_Value(str2int(yytext, 16)));return TOK_I32CONST;}
{OCTI32CONST}  { yylval->ast_node = new Ast_Node(I32CONST_, Basic_Value(str2int(yytext, 8)));return TOK_I32CONST;}
{DECI32CONST}  { yylval->ast_node = new Ast_Node(I32CONST_, Basic_Value(str2int(yytext, 10)));return TOK_I32CONST;}

{ID} {
    yylval->ast_node = new Ast_Node(ID_, yytext);
    return TOK_ID;
}

[/][/] { BEGIN(LINECOMMENT); }
<LINECOMMENT>[^\\\n]+
<LINECOMMENT>[\\]+/[^\n]
<LINECOMMENT>([\\][\n])+
<LINECOMMENT>[\n] { BEGIN(INITIAL); }

[/][*] { BEGIN(MULTICOMMENT); }
<MULTICOMMENT>[^*]+
<MULTICOMMENT>[*]+/[^/]
<MULTICOMMENT>[*][/] { BEGIN(INITIAL); }
%%
I32CONST_t str2int(const char* text, int base){
	char *endptr;
    errno = 0;
    return strtoll(text, &endptr, base);
}

F32CONST_t str2float(const char *text) {
    char *endptr;
    errno = 0;
    return strtod(text, &endptr);
}
