typedef union {
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
#define	TPLUS	285
#define	TMINUS	286
#define	TMUL	287
#define	TDIV	288


extern YYSTYPE yylval;
