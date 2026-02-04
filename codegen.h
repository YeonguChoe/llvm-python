#ifndef CODEGEN_H
#define CODEGEN_H

#include <map>

#include "ast.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constant.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "jit.h"

extern std::unique_ptr<llvm::LLVMContext> TheContext; // Tool set
extern std::unique_ptr<llvm::IRBuilder<> > Builder; // Generate IR
extern std::unique_ptr<llvm::Module> TheModule; // IR code container

extern std::unique_ptr<llvm::FunctionPassManager> TheFPM; // Container for Function Passes
extern std::unique_ptr<llvm::LoopAnalysisManager> TheLAM; // Loop analyzer
extern std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM; // Function analyzer
extern std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM; // Call graph SCC detector
extern std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM; // Module analyzer
extern std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC; // Pass debugger register
extern std::unique_ptr<llvm::StandardInstrumentations> TheSI; // Pass debugger toolkit

extern std::map<std::string, llvm::Value *> NamedValues; // Symbol table

extern std::unique_ptr<JIT> TheJit;

extern llvm::ExitOnError ExitOnErr;

extern std::map<std::string, std::unique_ptr<ASTNode::SignatureASTNode> > Signatures;

void InitializeModuleAndManagers();

llvm::Value *LogErrorV(const char *str);




#endif
