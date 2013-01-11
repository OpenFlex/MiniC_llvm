%option noyywrap
%option stack
%option outfile="lexer.cpp"

%{
#include <string>
#include "node.h"
#include "parser.h"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)

#define YY_NEVER_INTERACTIVE 1 
#define isatty _isatty

char linebuf[50];
unsigned int lineNo = 1;

extern void yyerror(char *s);

%}

%%

\n.*                    { 
                            strncpy(linebuf, yytext+1, sizeof(linebuf)); /* save the next line */
                            yyless(1);      /* give back all but the \n to rescan */                            
                            ++lineNo;
                        }

[ \t]					;
[a-zA-Z_][a-zA-Z0-9_]* 	SAVE_TOKEN; return IDENTIFIER;
[0-9]+\.[0-9]* 			SAVE_TOKEN; return DOUBLE_CONSTANT;
[0-9]+					SAVE_TOKEN; return INTEGER_CONSTANT;
"="						return TOKEN(EQUAL);
"=="					return TOKEN(CEQ);
"!="					return TOKEN(CNE);
"<"						return TOKEN(CLT);
"<="					return TOKEN(CLE);
">"						return TOKEN(CGT);
">="					return TOKEN(CGE);
"("						return TOKEN(LPAREN);
")"						return TOKEN(RPAREN);
"{"						return TOKEN(LBRACE);
"}"						return TOKEN(RBRACE);
"."						return TOKEN(DOT);
","						return TOKEN(COMMA);
"+"						return TOKEN(PLUS);
"-"						return TOKEN(MINUS);
"*"						return TOKEN(MUL);
"/"						return TOKEN(DIV);
";"                     return TOKEN(SEMICOLON);
.						printf("Unknown token!\n"); yyterminate();
%%
