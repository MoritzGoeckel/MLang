#include "LLVMRunner.h"

#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

LLVMRunner::LLVMRunner(std::unique_ptr<llvm::Module> module,
                       std::shared_ptr<llvm::LLVMContext>& context)
    : engine(nullptr), context(context), main(nullptr), isBroken(false) {
    isBroken = llvm::verifyModule(*module, &llvm::outs());
    if (isBroken) {
        return;
    }

    main = module->getFunction("main");
    engine = llvm::EngineBuilder(std::move(module)).create();
}

LLVMRunner::~LLVMRunner() {
    if (engine) delete engine;
}

bool LLVMRunner::getIsBroken() { return isBroken; }

bool LLVMRunner::addModule(const std::string& code) {
    // ParseIR
    // ParseIRFile

    llvm::SMDiagnostic error;
    std::unique_ptr<llvm::Module> module(
        llvm::parseIR({code, "Lib"}, error, *context));  // TODO name
    // if (error) error->dump();                           // TODO err handling
    return addModule(std::move(module));
}

bool LLVMRunner::addModule(std::unique_ptr<llvm::Module>&& module) {
    if (isBroken) return false;
    isBroken = llvm::verifyModule(*module, &llvm::outs());
    engine->addModule(std::move(module));
    return !isBroken;
}

Mlang::Result LLVMRunner::run() {
    if (isBroken) {
        return Mlang::Result(Mlang::Result::Signal::Failure)
            .addError("LLVM: Module invalid");
    }

    std::vector<llvm::GenericValue> emptyArgs;
    llvm::GenericValue gv = engine->runFunction(main, emptyArgs);

    if (main->getReturnType()->isIntegerTy()) {
        // llvm::outs() << gv.IntVal << '\n';
        return Mlang::Result(Mlang::Result::Signal::Success,
                             gv.IntVal.toString(10, true));
    }

    return Mlang::Result(Mlang::Result::Signal::Success);
}

