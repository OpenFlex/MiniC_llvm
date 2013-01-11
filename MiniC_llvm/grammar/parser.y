%{
	#include "node.h"
        #include <cstdio>
        #include <cstdlib>
	NBlock *programBlock; /* the top level root node of our final AST */

	extern int yylex();

	extern unsigned int lineNo;
	extern char *yytext;
	extern char linebuf[50];

    void yyerror(char *s)
    {
         printf("Line %d: %s at %s in this line:\n%s\n",
                lineNo, s, yytext, linebuf);
		 system("pause");
		 exit(1);
    }
%}

/* Represents the many different ways we can access our data */
%union {
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
	NVariableDeclaration *var_decl;
	std::vector<NVariableDeclaration*> *varvec;
	std::vector<NExpression*> *exprvec;
	std::string *string;
	int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> IDENTIFIER INTEGER_CONSTANT DOUBLE_CONSTANT
%token <token> EQUAL CEQ CNE CLT CLE CGT CGE
%token <token> LPAREN RPAREN LBRACE RBRACE COMMA DOT SEMICOLON
%token <token> PLUS MINUS MUL DIV
%token <token> INT FLOAT DOUBLE BOOL CHAR VOID

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> numeric expr 
%type <varvec> func_decl_args
%type <exprvec> call_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl
%type <token> comparison

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV

%start program

%%

program : stmts { programBlock = $1; }
		;
		
stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
	  | stmts stmt { $1->statements.push_back($<stmt>2); }
	  ;

stmt : var_decl SEMICOLON
     | func_decl
	 | expr { $$ = new NExpressionStatement(*$1); }
     ;

block : LBRACE stmts RBRACE { $$ = $2; }
	  | LBRACE RBRACE { $$ = new NBlock(); }
	  ;

var_decl : ident ident { $$ = new NVariableDeclaration(*$1, *$2); }
		 | ident ident EQUAL expr { $$ = new NVariableDeclaration(*$1, *$2, $4); }
		 ;
		
func_decl : ident ident LPAREN func_decl_args RPAREN block 
			{ $$ = new NFunctionDeclaration(*$1, *$2, *$4, *$6); delete $4; }
		  ;
	
func_decl_args : /*blank*/  { $$ = new VariableList(); }
		  | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
		  | func_decl_args COMMA var_decl { $1->push_back($<var_decl>3); }
		  ;

ident : IDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
	  ;

numeric : INTEGER_CONSTANT { $$ = new NInteger(atol($1->c_str())); delete $1; }
		| DOUBLE_CONSTANT { $$ = new NDouble(atof($1->c_str())); delete $1; }
		;
	
expr : ident EQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
	 | ident LPAREN call_args RPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
	 | ident { $<ident>$ = $1; }
	 | numeric
 	 | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | LPAREN expr RPAREN { $$ = $2; }
	 ;
	
call_args : /*blank*/  { $$ = new ExpressionList(); }
		  | expr { $$ = new ExpressionList(); $$->push_back($1); }
		  | call_args COMMA expr  { $1->push_back($3); }
		  ;

comparison : CEQ | CNE | CLT | CLE | CGT | CGE 
		   | PLUS | MINUS | MUL | DIV
		   ;

%%
