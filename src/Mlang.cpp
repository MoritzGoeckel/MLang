#include "Mlang.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "executer/LLVMRunner.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "parser/Parser.h"
#include "parser/Tokenizer.h"
#include "preprocessor/Preprocessor.h"
#include "transformer/HasUnknownTypes.h"
#include "transformer/ImplicitReturn.h"
#include "transformer/InfereIdentifierTypes.h"
#include "transformer/InfereParameterTypes.h"
#include "transformer/InstantiateFunctions.h"
#include "transformer/LLVMEmitter.h"

Mlang::Mlang() {}

Mlang::~Mlang() {}

void Mlang::init() {
    static bool isInitialized = false;
    if (!isInitialized) {
        llvm::InitializeNativeTarget();
        LLVMInitializeNativeAsmPrinter();
        LLVMInitializeNativeAsmParser();
        isInitialized = true;
    } else {
        return;
    }
}

void Mlang::shutdown() {
    static bool isShutdown = false;
    if (!isShutdown) {
        llvm::llvm_shutdown();
        isShutdown = true;
    } else {
        std::cout << "Error: Shutdown already called!" << std::endl;
    }
}

Mlang::Result Mlang::executeString(const std::string& theCode) {
    return execute("internal", theCode);
}

Mlang::Result Mlang::execute(const std::string& theFile,
                             const std::string& theCode) {
    Tokenizer tokenizer(theFile, theCode);

    auto tokens = tokenizer.getTokens();
    if (settings.showTokens) {
        for (auto token : tokens) {
            std::cout << token << " ";
        }
        std::cout << std::endl;
    }

    Parser parser(std::move(tokens));
    auto ast = parser.getAst();

    if (!ast) {
        return Mlang::Result(Mlang::Result::Signal::Failure)
            //.addError("Parsing failed:")
            .addError(parser.getError(theCode));
    }

    if (settings.showAbastractSyntaxTree) {
        std::cout << ast->toString() << std::endl;
    }

    {
        // Add implicit return
        ImplicitReturn impRet;
        impRet.process(ast);
    }

    {
        // Infere types
        HasUnknownTypes validator;
        size_t lastUnresolved = 0xfffffffff;

        while (true) {
            // Identifier types
            InfereIdentifierTypes identTypesWalker;
            identTypesWalker.process(ast);

            // Function types / params
            InfereParameterTypes paramTypesWalker;
            paramTypesWalker.process(ast);

            validator.process(ast);
            if (validator.hasTypeConflicts() ||
                validator.isAllTypesResolved() ||
                validator.getNumUnresolved() >= lastUnresolved) {
                // Conflicts occurred, all types are resolved or number of
                // resolved types did not increase -> Stop
                break;
            } else {
                lastUnresolved = validator.getNumUnresolved();
                validator.reset();
            }
        }

        const auto& aTypeErrors = validator.getErrors();
        if (!aTypeErrors.empty()) {
            // We only add one error
            return Mlang::Result(Mlang::Result::Signal::Failure)
                .addError("Type inference failed:\n" +
                          aTypeErrors.front().generateString(theCode));
        }

        if (!validator.isAllTypesResolved() || validator.hasTypeConflicts()) {
            throwConstraintViolated("Unresolved types and no errors");
        }

        if (settings.showInferedTypes) {
            std::cout << ast->toString() << std::endl;
        }
    }

    // TODO: Verify ast. E.g. scopes

    // Instantiate functions
    InstantiateFunctions instantiator(ast);

    // Do not use ast after this point!
    auto fns = instantiator.getFunctions();
    if (settings.showFunctions) {
        for (auto& fn : fns) {
            std::cout << fn.first << std::endl;
            std::cout << fn.second->AST::Node::toString() << std::endl;
            std::cout << std::endl;
        }
    }

    auto context = std::make_shared<llvm::LLVMContext>();
    LLVMEmitter emitter(fns, context);
    emitter.run();

    if (settings.showOptimizedModule) {
        emitter.print();
    }

    auto mod = emitter.getModule();
    LLVMRunner runner(std::move(mod), context);

    // TODO
    std::ifstream stream("lib/print.ll");
    std::stringstream strBuffer;
    strBuffer << stream.rdbuf();
    auto libSrc = strBuffer.str();
    std::cout << libSrc << std::endl;

    runner.addModule(libSrc);

    // TODO: Add lib. Keep context from emitter around

    // TODO: Have stdlib (+ - * / < > ==)
    // TODO: Use optimization passes

    return runner.run();
}

Mlang::Result Mlang::executeFile(std::string thePath) {
    std::ifstream stream(thePath);
    std::stringstream strBuffer;
    strBuffer << stream.rdbuf();

    auto fileContent = strBuffer.str();

    if (settings.showFileContent) {
        std::cout << "File: " << thePath << std::endl
                  << fileContent << std::endl;
    }

    if (!fileContent.empty()) {
        return execute(thePath, fileContent);
    } else {
        return Mlang::Result(Mlang::Result::Signal::Failure)
            .addError("File at " + thePath + " is empty");
    }
}

Mlang::Result::Result(Mlang::Result::Signal signal, const std::string& content)
    : signal(signal), content(content) {}

Mlang::Result::Result(Mlang::Result::Signal signal)
    : signal(signal), content("void") {}

Mlang::Result::operator Result::Signal() const { return signal; }

const std::string& Mlang::Result::getResult() const { return content; }

Mlang::Result& Mlang::Result::addError(const std::string& errorText) {
    itsErrors.push_back(errorText);
    return *this;
}

const std::vector<std::string>& Mlang::Result::getErrors() const {
    return itsErrors;
}

std::string Mlang::Result::getErrorString() const {
    std::string aOutput = "";
    for (const auto& aLine : itsErrors) aOutput += '\n' + aLine;
    return aOutput;
}

