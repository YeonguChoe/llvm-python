#ifndef JIT_H
#define JIT_H

#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/Mangling.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/SelfExecutorProcessControl.h"

class JIT {
    std::unique_ptr<llvm::orc::ExecutionSession> ES; // JIT system
    llvm::DataLayout DL; // Information on target device
    llvm::orc::MangleAndInterner Mangle; // unique name ensurer

    llvm::orc::IRCompileLayer CompileLayer; // LLVM IR -> Machine code generator
    llvm::orc::RTDyldObjectLinkingLayer ObjectLayer; // compiled code resolver

    llvm::orc::JITDylib &MainJD; // Lazy linker

public:
    // Constructor
    JIT(std::unique_ptr<llvm::orc::ExecutionSession> ES,
        llvm::orc::JITTargetMachineBuilder JTMB,
        llvm::DataLayout DL)
        : ES(std::move(ES)),
          DL(std::move(DL)),
          Mangle(*this->ES, this->DL),
          CompileLayer(
              *this->ES, ObjectLayer,
              std::make_unique<
                  llvm::orc::ConcurrentIRCompiler>(
                  std::move(JTMB))),
          ObjectLayer(*this->ES,
                      [](const llvm::MemoryBuffer &) {
                          return std::make_unique<llvm::SectionMemoryManager>();
                      }
          ),
          MainJD(this->ES->createBareJITDylib("<main>")) {
        MainJD.addGenerator(
            llvm::cantFail(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix())));
        if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
            ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
            ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
        }
    }

    // Destructor
    ~JIT() {
        if (auto error = ES->endSession()) {
            ES->reportError(std::move(error));
        }
    }

    // Factory method
    static llvm::Expected<std::unique_ptr<JIT> > Create() {
        auto EPC = llvm::orc::SelfExecutorProcessControl::Create();
        if (!EPC) {
            return EPC.takeError();
        }

        auto ES = std::make_unique<llvm::orc::ExecutionSession>(std::move(*EPC));
        llvm::orc::JITTargetMachineBuilder JTMB(ES->getExecutorProcessControl().getTargetTriple());
        auto DL = JTMB.getDefaultDataLayoutForTarget();

        if (!DL) {
            return DL.takeError();
        }

        return std::make_unique<JIT>(
            std::move(ES),
            std::move(JTMB),
            std::move(*DL)
        );
    }

    // Getter
    const llvm::DataLayout &getDataLayout() const {
        return DL;
    }

    llvm::orc::JITDylib &getMainJITDylib() {
        return MainJD;
    }


    // Register LLVM IR to JIT
    llvm::Error addModule(llvm::orc::ThreadSafeModule TSM, llvm::orc::ResourceTrackerSP RT = nullptr) {
        if (RT == nullptr) {
            RT = MainJD.getDefaultResourceTracker();
        }
        return CompileLayer.add(RT, std::move(TSM));
    }

    // Symbol (Function, Variable) information
    llvm::Expected<llvm::orc::ExecutorSymbolDef> lookup(llvm::StringRef SymbolName) {
        return ES->lookup({&MainJD}, Mangle(SymbolName.str()));
    }
};


#endif
