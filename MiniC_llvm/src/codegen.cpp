#include "node.h"
#include "codegen.h"
#include "parser.h"

using namespace std;

/* Compile the AST into a module */
void CodeGenContext::generateCode(Block& root)
{
    std::cout << "Generating code...\n";
    
    /* Create the top level interpreter function to call as entry */
    vector<llvm::Type*> argTypes;
    llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), llvm::makeArrayRef(argTypes), false);
    mainFunction = llvm::Function::Create(ftype, llvm::GlobalValue::InternalLinkage, "main", module);
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", mainFunction, 0);
    g_Builder.SetInsertPoint(bblock);

    /* Push a new variable/block context */
    pushBlock(bblock);
    llvm::Value* pRetVal = root.codeGen(*this); /* emit bytecode for the toplevel block */
    g_Builder.CreateRet(pRetVal);
    popBlock();
    
    /* Print the bytecode in a human-readable format 
       to see if our program compiled properly
     */
    std::cout << "Code is generated.\n";
    llvm::PassManager pm;
    pm.add(llvm::createPrintModulePass(&llvm::outs()));
    pm.add(llvm::createBasicAliasAnalysisPass());
    
    pm.add(llvm::createInstructionCombiningPass());

    pm.add(llvm::createReassociatePass());

    pm.add(llvm::createGVNPass());

    pm.add(llvm::createCFGSimplificationPass());
    pm.add(llvm::createPrintModulePass(&llvm::outs()));

    pm.run(*module);
}

/* Executes the AST by running the main function */
llvm::GenericValue CodeGenContext::runCode() {
    std::cout << "Running code...\n";
    llvm::ExecutionEngine *ee = llvm::EngineBuilder(module).create();
    vector<llvm::GenericValue> noargs;
    llvm::GenericValue v = ee->runFunction(mainFunction, noargs);
    std::cout << "Code was run.\n";
    return v;
}

/* Returns an LLVM type based on the identifier */
static llvm::Type *typeOf(const Identifier& type) 
{
    if (type.name.compare("int") == 0) {
        return llvm::Type::getInt32Ty(llvm::getGlobalContext());
    }
    else if (type.name.compare("double") == 0) {
        return llvm::Type::getDoubleTy(llvm::getGlobalContext());
    }
    return llvm::Type::getVoidTy(llvm::getGlobalContext());
}

/* -- Code Generation -- */

llvm::Value* ConstInt::codeGen(CodeGenContext& context)
{
    std::cout << "Creating integer: " << value << endl;
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvm::getGlobalContext()), value, true);
}

llvm::Value* ConstDouble::codeGen(CodeGenContext& context)
{
    std::cout << "Creating double: " << value << endl;
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), value);
}

llvm::Value* Identifier::codeGen(CodeGenContext& context)
{
    std::cout << "Creating identifier reference: " << name << endl;
    
    if (context.locals().find(name) == context.locals().end()) 
    {
        std::cerr << "undeclared variable " << name << endl;
        return NULL;
    }
    
    return new llvm::LoadInst(context.locals()[name], "", false, context.currentBlock());
}

llvm::Value* MethodCall::codeGen(CodeGenContext& context)
{
    llvm::Function *function = context.module->getFunction(id.name.c_str());
    if (function == NULL) 
    {
        std::cerr << "no such function " << id.name << endl;
    }
    
    std::vector<llvm::Value*> args;
    ExpressionList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) 
    {
        args.push_back((**it).codeGen(context));
    }
    
    llvm::CallInst *call = llvm::CallInst::Create(function, llvm::makeArrayRef(args), "", context.currentBlock());
    std::cout << "Creating method call: " << id.name << endl;
    return call;
}

llvm::Value* BinaryOp::codeGen(CodeGenContext& context)
{
    llvm::Value* L = lhs.codeGen(context);
    llvm::Value* R = rhs.codeGen(context);

    std::cout << "Creating binary operation " << op << endl;
    
    if (L == NULL || R == NULL)
        return NULL;

    llvm::Value* pInst = NULL;

    switch (op) 
    {
        case PLUS: 
            pInst = g_Builder.CreateAdd(L, R);
            break;
        case MINUS:
            pInst = g_Builder.CreateFSub(L, R);
            break;
        case MUL:
            pInst = g_Builder.CreateFMul(L, R);
            break;
        case DIV:
            pInst = g_Builder.CreateFDiv(L, R);
            break;				
        /* TODO comparison */
    }

    return pInst;
}

llvm::Value* AssignmentExpr::codeGen(CodeGenContext& context)
{
    std::cout << "Creating assignment for " << lhs.name << endl;
    
    if (context.locals().find(lhs.name) == context.locals().end()) 
    {
        std::cerr << "undeclared variable " << lhs.name << endl;
        return NULL;
    }
    
    return g_Builder.CreateStore(rhs.codeGen(context), context.locals()[lhs.name], false);
}

llvm::Value* Block::codeGen(CodeGenContext& context)
{
    StatementList::const_iterator it;
    llvm::Value *last = NULL;
    
    for (it = statements.begin(); it != statements.end(); it++) 
    {
        std::cout << "Generating code for " << typeid(**it).name() << endl;
        last = (**it).codeGen(context);
    }
    
    std::cout << "Creating block" << endl;
    return last;
}

llvm::Value* ExprStmt::codeGen(CodeGenContext& context)
{
    std::cout << "Generating code for " << typeid(expression).name() << endl;
    return expression.codeGen(context);
}

llvm::Value* VarDecl::codeGen(CodeGenContext& context)
{
    std::cout << "Creating variable declaration " << type.name << " " << id.name << endl;
    llvm::AllocaInst *alloc = g_Builder.CreateAlloca(typeOf(type));
    alloc->setName(id.name.c_str());
    context.locals()[id.name] = alloc;

    if (assignmentExpr != NULL) 
    {
        AssignmentExpr assn(id, *assignmentExpr);
        llvm::Value* assnVal = assn.codeGen(context);
    }

    return alloc;
}

llvm::Value* FuncDecl::codeGen(CodeGenContext& context)
{
    vector<llvm::Type*> argTypes;
    VariableList::const_iterator it;
    
    for (it = arguments.begin(); it != arguments.end(); it++) 
    {
        argTypes.push_back(typeOf((**it).type));
    }
    
    llvm::FunctionType *ftype = llvm::FunctionType::get(typeOf(type), llvm::makeArrayRef(argTypes), false);
    llvm::Function *function = llvm::Function::Create(ftype, llvm::GlobalValue::InternalLinkage, id.name.c_str(), context.module);
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function, 0);
    g_Builder.SetInsertPoint(bblock);
    context.pushBlock(bblock);

    for (it = arguments.begin(); it != arguments.end(); it++) 
    {
        (**it).codeGen(context);
    }
    
    llvm::Value* pRetVal = block.codeGen(context);
    g_Builder.CreateRet(pRetVal);

    context.popBlock();

    llvm::BasicBlock* pPrevBlock = context.currentBlock();
    g_Builder.SetInsertPoint(pPrevBlock);

    std::cout << "Creating function: " << id.name << endl;
    return function;
}

llvm::Value* IfExpr::codeGen(CodeGenContext& context)
{
    llvm::Value* pCond = expression.codeGen(context);
    if (pCond == NULL)
        return NULL;

    pCond = g_Builder.CreateFCmpONE(pCond,
                                    llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(0.0)),
                                    "ifcond");

    llvm::Function* pFunction = g_Builder.GetInsertBlock()->getParent();

    // Create blocks for then and else cases. Insert the 'then' block at the end of function
    llvm::BasicBlock* pThenBB  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", pFunction);
    llvm::BasicBlock* pElseBB  = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else");
    llvm::BasicBlock* pMergeBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "ifcont");

    g_Builder.CreateCondBr(pCond, pThenBB, pElseBB);
}
