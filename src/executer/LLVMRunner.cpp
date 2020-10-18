#include "LLVMRunner.h"

LLVMRunner::LLVMRunner(std::unique_ptr<llvm::Module> module)
    : engine(nullptr), main(nullptr), isBroken(false) {
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

LLVMRunner::Result LLVMRunner::run() {
    if (isBroken) {
        return LLVMRunner::Result::InvalidModule;
    }

    std::vector<llvm::GenericValue> emptyArgs;
    llvm::GenericValue gv = engine->runFunction(main, emptyArgs);

    // TODO, maybe show other return types
    if (main->getReturnType()->isIntegerTy()) {
        llvm::outs() << "Result: " << gv.IntVal << '\n';
    }

    return LLVMRunner::Result::Success;
}
