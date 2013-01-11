typedef union {
	Node       *node;
	Block      *block;
	Expr       *expr;
	Stmt       *stmt;
	Identifier *ident;
	VarDecl    *var_decl;
	IfExpr     *if_expr;
	
	std::vector<VarDecl*> *varvec;
	std::vector<Expr*> *exprvec;
	std::string *string;
	int token;
} YYSTYPE;
#define	IDENTIFIER	258
#define	INTEGER_CONSTANT	259
#define	DOUBLE_CONSTANT	260
#define	EQUAL	261
#define	CEQ	262
#define	CNE	263
#define	CLT	264
#define	CLE	265
#define	CGT	266
#define	CGE	267
#define	LPAREN	268
#define	RPAREN	269
#define	LBRACE	270
#define	RBRACE	271
#define	COMMA	272
#define	DOT	273
#define	SEMICOLON	274
#define	PLUS	275
#define	MINUS	276
#define	MUL	277
#define	DIV	278
#define	INT	279
#define	FLOAT	280
#define	DOUBLE	281
#define	BOOL	282
#define	CHAR	283
#define	VOID	284
#define	IF	285


extern YYSTYPE yylval;
