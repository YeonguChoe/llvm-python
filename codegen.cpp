#include "codegen.h"
#include "ast.h"
#include "parser.h"


std::unique_ptr<llvm::LLVMContext> TheContext; // Tool set
std::unique_ptr<llvm::IRBuilder<> > Builder; // Generate IR
std::unique_ptr<llvm::Module> TheModule; // IR code container
std::map<std::string, llvm::Value *> NamedValues; // Symbol table

// Manager
// Transform pass
std::unique_ptr<llvm::FunctionPassManager> TheFPM; // Container for Function Passes

// Analysis pass
std::unique_ptr<llvm::LoopAnalysisManager> TheLAM; // Loop analyzer
std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM; // Function analyzer
std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM; // Call graph SCC detector
std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM; // Module analyzer

// Debugger
std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC; // Pass debugger register
std::unique_ptr<llvm::StandardInstrumentations> TheSI; // Pass debugger toolkit

std::unique_ptr<JIT> TheJit;

std::map<std::string, std::unique_ptr<ASTNode::SignatureASTNode> > Signatures;

llvm::ExitOnError ExitOnErr;

void InitializeModuleAndManagers() {
    // Context, Builder, Module
    TheContext = std::make_unique<llvm::LLVMContext>();
    Builder = std::make_unique<llvm::IRBuilder<> >(*TheContext);
    TheModule = std::make_unique<llvm::Module>("JIT", *TheContext);
    TheModule->setDataLayout(TheJit->getDataLayout());

    // Manager
    TheFPM = std::make_unique<llvm::FunctionPassManager>();
    TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
    TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
    TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
    TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();
    ThePIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
    TheSI = std::make_unique<llvm::StandardInstrumentations>(*TheContext, true);
    TheSI->registerCallbacks(*ThePIC, TheMAM.get());

    // Add transform passes
    TheFPM->addPass(llvm::InstCombinePass());
    TheFPM->addPass(llvm::ReassociatePass());
    TheFPM->addPass(llvm::GVNPass());
    TheFPM->addPass(llvm::SimplifyCFGPass());

    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(*TheMAM);
    PB.registerFunctionAnalyses(*TheFAM);
    PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);
}

llvm::Value *LogErrorV(const char *str) {
    LogError(str);
    return nullptr;
}

llvm::Function *getFunction(std::string Name) {
    auto *Function = TheModule->getFunction(Name);
    if (Function != nullptr) {
        return Function;
    }
    auto FI = Signatures.find(Name);
    if (FI != Signatures.end()) {
        return FI->second->codegen();
    }
    return nullptr;
}

llvm::Value *ASTNode::NumberExpressionASTNode::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(value));
}

llvm::Value *ASTNode::VariableExpressionASTNode::codegen() {
    llvm::Value *V = NamedValues[Name];
    if (V == nullptr) {
        LogErrorV("Unknown variable name");
    }
    return V;
}

llvm::Value *ASTNode::BinaryExpressionASTNode::codegen() {
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();

    if (L == nullptr or R == nullptr) {
        return nullptr;
    }

    switch (Operator) {
        case '+':
            return Builder->CreateFAdd(L, R, "addtmp");
        case '-':
            return Builder->CreateSub(L, R, "subtmp");
        case '*':
            return Builder->CreateFMul(L, R, "multmp");
        case '<':
            L = Builder->CreateFCmpULT(L, R, "cmptmp");
            return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
        default:
            return LogErrorV("invalid binary operator");
    }
}

llvm::Value *ASTNode::FunctionCallExpressionASTNode::codegen() {
    llvm::Function *CalleeFunction = getFunction(Callee);

    if (CalleeFunction == nullptr) {
        return LogErrorV("Unknown function referenced");
    }

    if (CalleeFunction->arg_size() != Arguments.size()) {
        return LogErrorV("Incorrect number of arguments passed");
    }

    std::vector<llvm::Value *> ArgumentsVector;

    for (unsigned int i = 0, e = Arguments.size(); i != e; ++i) {
        ArgumentsVector.push_back(Arguments[i]->codegen());
        if (ArgumentsVector.back() == nullptr) {
            return nullptr;
        }
    }
    return Builder->CreateCall(CalleeFunction, ArgumentsVector, "calltmp");
}

llvm::Value *ASTNode::IfExpressionASTNode::codegen() {
    llvm::Value *ConditionValue = Condition->codegen();
    if (ConditionValue == nullptr) {
        return nullptr;
    }

    ConditionValue = Builder->CreateFCmpONE(ConditionValue, llvm::ConstantFP::get(*TheContext, llvm::APFloat(0.0)),
                                            "ifcond");

    // Create Then, Else, Continue Blocks
    llvm::Function *TheFunction = Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");
    Builder->CreateCondBr(ConditionValue, ThenBB, ElseBB);

    // Write Then block
    Builder->SetInsertPoint(ThenBB);
    llvm::Value *ThenValue = Then->codegen();
    if (ThenValue == nullptr) {
        return nullptr;
    }
    Builder->CreateBr(MergeBB);

    // Save Then block starting point
    ThenBB = Builder->GetInsertBlock();

    // Write Else block
    TheFunction->insert(TheFunction->end(), ElseBB);
    Builder->SetInsertPoint(ElseBB);
    llvm::Value *ElseValue = Else->codegen();

    if (ElseValue == nullptr) {
        return nullptr;
    }

    Builder->CreateBr(MergeBB);
    ElseBB = Builder->GetInsertBlock();

    // Write Merge block
    TheFunction->insert(TheFunction->end(), MergeBB);
    Builder->SetInsertPoint(MergeBB);
    llvm::PHINode *PN = Builder->CreatePHI(llvm::Type::getDoubleTy(*TheContext), 2, "iftmp");

    PN->addIncoming(ThenValue, ThenBB);
    PN->addIncoming(ElseValue, ElseBB);

    return PN;
}

llvm::Function *ASTNode::SignatureASTNode::codegen() {
    std::vector<llvm::Type *> Doubles(Arguments.size(), llvm::Type::getDoubleTy(*TheContext));
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

    unsigned int Index = 0;
    for (auto &Argument: F->args()) {
        Argument.setName(Arguments[Index++]);
    }
    return F;
}

llvm::Function *ASTNode::FunctionASTNode::codegen() {
    auto &P = *Signature;
    Signatures[Signature->getName()] = std::move(Signature);

    llvm::Function *TheFunction = TheModule->getFunction(P.getName());

    if (TheFunction == nullptr) {
        TheFunction = P.codegen();
    }

    if (TheFunction == nullptr) {
        return nullptr;
    }

    if (TheFunction->empty() == false) {
        LogErrorV("Function cannot be redefined.");
        return nullptr;
    }

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    NamedValues.clear();
    for (auto &Argument: TheFunction->args()) {
        NamedValues[Argument.getName().str()] = &Argument;
    }

    llvm::Value *ReturnValue = Body->codegen();
    if (ReturnValue == nullptr) {
        TheFunction->eraseFromParent();
        return nullptr;
    }
    Builder->CreateRet(ReturnValue);
    llvm::verifyFunction(*TheFunction);
    TheFPM->run(*TheFunction, *TheFAM);
    return TheFunction;
}
