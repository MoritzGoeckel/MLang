#include "Mlang.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "MGrammarBaseListener.h"
#include "MGrammarBaseVisitor.h"
#include "MGrammarLexer.h"
#include "MGrammarParser.h"
#include "antlr4-runtime.h"
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
#include "parser/PrintVisitor.h"
#include "parser/PtToAstVisitor.h"
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

Mlang::Signal Mlang::executeString(std::string theCode) {
    Preprocessor::run(theCode);

    // ------------------- ANTLR ------------------
    antlr4::ANTLRInputStream input(theCode.c_str(), theCode.size());
    MGrammar::MGrammarLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    tokens.fill();

    if (settings.showTokens) {
        for (auto token : tokens.getTokens()) {
            std::cout << token->toString() << std::endl;
        }
    }

    // Generate parse tree
    MGrammar::MGrammarParser parser(&tokens);
    // const std::vector<std::string> &ruleNames = parser.getRuleNames();
    antlr4::tree::ParseTree *tree = parser.r();

    if (settings.showParseTree) {
        std::cout << tree->toStringTree(&parser) << std::endl << std::endl;
    }

    if (settings.showPrettyParseTree) {
        std::cout << "Pretty Parse Tree:" << std::endl;
        PrintVisitor visitor;
        visitor.visit(tree);
        std::cout << visitor.toString();
    }

    // Create ast
    PtToAstVisitor visitor;
    visitor.visit(tree);
    auto ast = visitor.getAST();

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

        if (!validator.isAllTypesResolved()) {
            // TODO: Output messages from last run
            std::cout << "Unresolved types: " << validator.getNumUnresolved()
                      << "!" << std::endl;

            return Mlang::Signal::Failure;
        }

        if (validator.hasTypeConflicts()) {
            // TODO: Output messages from last run
            std::cout << "Conflicting types: " << validator.getNumConflicts()
                      << "!" << std::endl;

            return Mlang::Signal::Failure;
        }

        if (settings.showInferedTypes) {
            std::cout << ast->toString() << std::endl;
        }
    }

    // Instantiate functions
    InstantiateFunctions instantiator(ast);
    // Do not use ast after this point
    auto fns = instantiator.getFunctions();
    if (settings.showFunctions) {
        for (auto &fn : fns) {
            std::cout << fn.first << std::endl;
            std::cout << fn.second->AST::Node::toString() << std::endl;
            std::cout << std::endl;
        }
    }

    LLVMEmitter emitter(fns);
    emitter.run();
    emitter.print();

    auto mod = emitter.getModule();
    LLVMRunner runner(std::move(mod));
    if (runner.run() != LLVMRunner::Result::Success) {
        return Mlang::Signal::Failure;
    }

    // TODO: Have stdlib (+ - * / < > ==)
    // TODO: Implement operator precedence transformer
    // TODO: Use return instead of stack in PtToAST
    // TODO: Use optimization passes

    return Mlang::Signal::Success;
}

Mlang::Signal Mlang::executeFile(std::string thePath) {
    std::ifstream stream(thePath);
    std::stringstream strBuffer;
    strBuffer << stream.rdbuf();

    auto fileContent = strBuffer.str();

    if (settings.showFileContent) {
        std::cout << "File: " << thePath << std::endl
                  << fileContent << std::endl;
    }

    if (!fileContent.empty()) {
        return executeString(fileContent);
    } else {
        std::cout << "File empty: " << thePath << std::endl;
        return Mlang::Signal::Failure;
    }
}
