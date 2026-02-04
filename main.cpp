#include <iostream>
#include "parser.h"
#include "codegen.h"

#include "llvm/Support/TargetSelect.h"

int main(int argc, char *argv[]) {
    // Set target
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    fprintf(stderr, ">>> ");
    getNextToken();
    TheJit = ExitOnErr(JIT::Create());
    InitializeModuleAndManagers();
    MainLoop();
    return 0;
}
