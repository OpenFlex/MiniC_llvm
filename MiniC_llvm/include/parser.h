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
#define	TIDENTIFIER	258
#define	TINTEGER	259
#define	TDOUBLE	260
#define	TCEQ	261
#define	TCNE	262
#define	TCLT	263
#define	TCLE	264
#define	TCGT	265
#define	TCGE	266
#define	TEQUAL	267
#define	TLPAREN	268
#define	TRPAREN	269
#define	TLBRACE	270
#define	TRBRACE	271
#define	TCOMMA	272
#define	TDOT	273
#define	TPLUS	274
#define	TMINUS	275
#define	TMUL	276
#define	TDIV	277


extern YYSTYPE yylval;
