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
