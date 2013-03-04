#pragma once

#include <stack>
#include <typeinfo>

#include <llvm\Config\config.h>
#if defined(LLVM_VERSION_MAJOR) && LLVM_VERSION_MAJOR == 3 && LLVM_VERSION_MINOR > 2 
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#else
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/Type.h>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/IRBuilder.h>
#include <llvm/Instructions.h>
#include <llvm/CallingConv.h>
#endif

#include <llvm/PassManager.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/Support/raw_ostream.h>

class Block;
static llvm::IRBuilder<> g_Builder(llvm::getGlobalContext());

class CodeGenBlock 
{
public:
    llvm::BasicBlock *block;
    std::map<std::string, llvm::Value*> locals;
};

class CodeGenContext 
{
    std::stack<CodeGenBlock *> blocks;
    llvm::Function *mainFunction;

public:
    llvm::Module *module;
    CodeGenContext() { module = new llvm::Module("main", llvm::getGlobalContext()); }
    
    void generateCode(Block& root);
    llvm::GenericValue runCode();
    std::map<std::string, llvm::Value*>& locals() { return blocks.top()->locals; }
    llvm::BasicBlock *currentBlock() { return blocks.top()->block; }
    void pushBlock(llvm::BasicBlock *block) { blocks.push(new CodeGenBlock()); blocks.top()->block = block; }
    void popBlock() { CodeGenBlock *top = blocks.top(); blocks.pop(); delete top; }
};
