#include "node.h"
#include "codegen.h"
#include "parser.h"

using namespace std;

/* Compile the AST into a module */
void CodeGenContext::generateCode(Block& root)
{
	std::cout << "Generating code...\n";
	
	/* Create the top level interpreter function to call as entry */
	vector<Type*> argTypes;
	FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), makeArrayRef(argTypes), false);
	mainFunction = Function::Create(ftype, GlobalValue::InternalLinkage, "main", module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", mainFunction, 0);
	g_Builder.SetInsertPoint(bblock);

	/* Push a new variable/block context */
	pushBlock(bblock);
	Value* pRetVal = root.codeGen(*this); /* emit bytecode for the toplevel block */
	g_Builder.CreateRet(pRetVal);
	popBlock();
	
	/* Print the bytecode in a human-readable format 
	   to see if our program compiled properly
	 */
	std::cout << "Code is generated.\n";
	PassManager pm;
	pm.add(createPrintModulePass(&outs()));
	pm.add(createBasicAliasAnalysisPass());
	
	pm.add(createInstructionCombiningPass());

	pm.add(createReassociatePass());

	pm.add(createGVNPass());

	pm.add(createCFGSimplificationPass());
	pm.add(createPrintModulePass(&outs()));

	pm.run(*module);
}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {
	std::cout << "Running code...\n";
	ExecutionEngine *ee = EngineBuilder(module).create();
	vector<GenericValue> noargs;
	GenericValue v = ee->runFunction(mainFunction, noargs);
	std::cout << "Code was run.\n";
	return v;
}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(const Identifier& type) 
{
	if (type.name.compare("int") == 0) {
		return Type::getInt32Ty(getGlobalContext());
	}
	else if (type.name.compare("double") == 0) {
		return Type::getDoubleTy(getGlobalContext());
	}
	return Type::getVoidTy(getGlobalContext());
}

/* -- Code Generation -- */

Value* ConstInt::codeGen(CodeGenContext& context)
{
	std::cout << "Creating integer: " << value << endl;
	return ConstantInt::get(Type::getInt32Ty(getGlobalContext()), value, true);
}

Value* ConstDouble::codeGen(CodeGenContext& context)
{
	std::cout << "Creating double: " << value << endl;
	return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

Value* Identifier::codeGen(CodeGenContext& context)
{
	std::cout << "Creating identifier reference: " << name << endl;
	
    if (context.locals().find(name) == context.locals().end()) 
    {
		std::cerr << "undeclared variable " << name << endl;
		return NULL;
	}
	
    return new LoadInst(context.locals()[name], "", false, context.currentBlock());
}

Value* MethodCall::codeGen(CodeGenContext& context)
{
	Function *function = context.module->getFunction(id.name.c_str());
	if (function == NULL) 
    {
		std::cerr << "no such function " << id.name << endl;
	}
	
    std::vector<Value*> args;
	ExpressionList::const_iterator it;
	for (it = arguments.begin(); it != arguments.end(); it++) 
    {
		args.push_back((**it).codeGen(context));
	}
	
    CallInst *call = CallInst::Create(function, makeArrayRef(args), "", context.currentBlock());
	std::cout << "Creating method call: " << id.name << endl;
	return call;
}

Value* BinaryOp::codeGen(CodeGenContext& context)
{
    Value* L = lhs.codeGen(context);
    Value* R = rhs.codeGen(context);

	std::cout << "Creating binary operation " << op << endl;
	
    if (L == NULL || R == NULL)
        return NULL;

    Value* pInst = NULL;

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

Value* AssignmentExpr::codeGen(CodeGenContext& context)
{
	std::cout << "Creating assignment for " << lhs.name << endl;
	
    if (context.locals().find(lhs.name) == context.locals().end()) 
    {
		std::cerr << "undeclared variable " << lhs.name << endl;
		return NULL;
	}
	
    return g_Builder.CreateStore(rhs.codeGen(context), context.locals()[lhs.name], false);
}

Value* Block::codeGen(CodeGenContext& context)
{
	StatementList::const_iterator it;
	Value *last = NULL;
	
    for (it = statements.begin(); it != statements.end(); it++) 
    {
		std::cout << "Generating code for " << typeid(**it).name() << endl;
		last = (**it).codeGen(context);
	}
	
    std::cout << "Creating block" << endl;
	return last;
}

Value* ExprStmt::codeGen(CodeGenContext& context)
{
	std::cout << "Generating code for " << typeid(expression).name() << endl;
	return expression.codeGen(context);
}

Value* VarDecl::codeGen(CodeGenContext& context)
{
	std::cout << "Creating variable declaration " << type.name << " " << id.name << endl;
    AllocaInst *alloc = g_Builder.CreateAlloca(typeOf(type));
    alloc->setName(id.name.c_str());
	context.locals()[id.name] = alloc;

    if (assignmentExpr != NULL) 
    {
		AssignmentExpr assn(id, *assignmentExpr);
		Value* assnVal = assn.codeGen(context);
	}

	return alloc;
}

Value* FuncDecl::codeGen(CodeGenContext& context)
{
	vector<Type*> argTypes;
	VariableList::const_iterator it;
	
    for (it = arguments.begin(); it != arguments.end(); it++) 
    {
		argTypes.push_back(typeOf((**it).type));
	}
	
    FunctionType *ftype = FunctionType::get(typeOf(type), makeArrayRef(argTypes), false);
	Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);
	BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);
    g_Builder.SetInsertPoint(bblock);
	context.pushBlock(bblock);

	for (it = arguments.begin(); it != arguments.end(); it++) 
    {
		(**it).codeGen(context);
	}
	
	Value* pRetVal = block.codeGen(context);
    g_Builder.CreateRet(pRetVal);

	context.popBlock();

	BasicBlock* pPrevBlock = context.currentBlock();
	g_Builder.SetInsertPoint(pPrevBlock);

	std::cout << "Creating function: " << id.name << endl;
	return function;
}

Value* IfExpr::codeGen(CodeGenContext& context)
{
    Value* pCond = expression.codeGen(context);
    if (pCond == NULL)
        return NULL;

    pCond = g_Builder.CreateFCmpONE(pCond,
                                    ConstantFP::get(getGlobalContext(), APFloat(0.0)),
                                    "ifcond");

    Function* pFunction = g_Builder.GetInsertBlock()->getParent();

    // Create blocks for then and else cases. Insert the 'then' block at the end of function
    BasicBlock* pThenBB  = BasicBlock::Create(getGlobalContext(), "then", pFunction);
    BasicBlock* pElseBB  = BasicBlock::Create(getGlobalContext(), "else");
    BasicBlock* pMergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");

    g_Builder.CreateCondBr(pCond, pThenBB, pElseBB);
}
