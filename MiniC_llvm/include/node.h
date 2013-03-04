#ifndef __NODE__
#define __NODE__

#include <iostream>
#include <vector>

#include <llvm\Config\config.h>
#if defined(LLVM_VERSION_MAJOR) && LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR > 2 
#include <llvm/IR/Value.h>
#else
#include <llvm/Value.h>
#endif

class CodeGenContext;
class Stmt;
class Expr;
class VarDecl;

typedef std::vector<Stmt*> StatementList;
typedef std::vector<Expr*> ExpressionList;
typedef std::vector<VarDecl*> VariableList;

class Node
{
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(CodeGenContext& context) { return NULL; }
};

class Expr : public Node
{
};

class Stmt : public Node
{
};

class ConstInt : public Expr
{
public:
    long long value;
    ConstInt(long long value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class ConstDouble : public Expr
{
public:
    double value;
    ConstDouble(double value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Identifier : public Expr
{
public:
    std::string name;
    Identifier(const std::string& name) : name(name) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class MethodCall : public Expr 
{
public:
    const Identifier& id;
    ExpressionList arguments;
    MethodCall(const Identifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }
    MethodCall(const Identifier& id) : id(id) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class BinaryOp : public Expr
{
public:
    int op;
    Expr& lhs;
    Expr& rhs;
    BinaryOp(Expr& lhs, int op, Expr& rhs) :
        lhs(lhs), rhs(rhs), op(op) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class AssignmentExpr : public Expr
{
public:
    Identifier& lhs;
    Expr& rhs;
    AssignmentExpr(Identifier& lhs, Expr& rhs) : 
        lhs(lhs), rhs(rhs) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class Block : public Expr
{
public:
    StatementList statements;
    Block() { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class ExprStmt : public Stmt
{
public:
    Expr& expression;
    ExprStmt(Expr& expression) : 
        expression(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class VarDecl : public Stmt
{
public:
    const Identifier& type;
    Identifier& id;
    Expr *assignmentExpr;
    VarDecl(const Identifier& type, Identifier& id) :
        type(type), id(id), assignmentExpr(NULL) { }
    VarDecl(const Identifier& type, Identifier& id, Expr *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class FuncDecl : public Stmt
{
public:
    const Identifier& type;
    const Identifier& id;
    VariableList arguments;
    Block& block;
    FuncDecl(const Identifier& type, const Identifier& id, 
            const VariableList& arguments, Block& block) :
        type(type), id(id), arguments(arguments), block(block) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class IfExpr : public ExprStmt
{
public:
    Block*      m_pBlock;

    IfExpr(Expr* pCondExpr, Block* pBlock)
        : ExprStmt(*pCondExpr),
          m_pBlock(pBlock)
    {}

    virtual llvm::Value* codeGen(CodeGenContext& context);
};

#endif