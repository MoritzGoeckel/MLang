#include "Mlang.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "MGrammarBaseListener.h"
#include "MGrammarBaseVisitor.h"
#include "MGrammarLexer.h"
#include "MGrammarParser.h"
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
#include "transformer/ImplicitReturn.h"
#include "transformer/InfereIdentifierTypes.h"

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
    antlr4::ANTLRInputStream input(theCode);
    MGrammar::MGrammarLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);

    tokens.fill();

    if (settings.showTokens) {
        for (auto token : tokens.getTokens()) {
            std::cout << token->toString() << std::endl;
        }
    }

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

    if (settings.showAbastractSyntaxTree) {
        std::cout << "Abstract Syntax Tree:" << std::endl;
        PtToAstVisitor visitor;
        visitor.visit(tree);
        auto ast = visitor.getAST();
        std::cout << ast->toString() << std::endl;

        {
            ImplicitReturn impRet;
            impRet.process(ast);
            std::cout << "Transformed AST:" << std::endl;
            std::cout << ast->toString() << std::endl;
        }

        if (settings.infereTypes) {
            // General types
            ast->infereDataType();

            // Identifier types
            InfereIdentifierTypes identTypesWalker;
            identTypesWalker.process(ast);

            // Re run general types
            ast->infereDataType();

            // Function types (params)
            // TODO

            // Output
            std::cout << "Infered types:" << std::endl;
            std::cout << ast->toString() << std::endl;
        }
    }

    // TODO: Infere types
    // TODO: Implement operator precedence transformer
    // TODO: Use return instead of stack in PtToAST
    // TODO: Generate LLVM
    // TODO: Have stdlib (+ - * / < > ==)

    /*
    // Run listener
    MListener listener;
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    */

    // ------------------- LLVM ------------------
    llvm::LLVMContext Context;

    // Create some module to put our function into it.
    auto Owner = std::make_unique<llvm::Module>("test", Context);
    llvm::Module *M = Owner.get();

    // Create the add1 function entry and insert this entry into module M.
    // The function will have a return type of "int" and take an argument of
    // "int".
    llvm::Function *Add1F = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(Context),
                                {llvm::Type::getInt32Ty(Context)}, false),
        llvm::Function::ExternalLinkage, "add1", M);

    // Add a basic block to the function. As before, it automatically
    // inserts because of the last argument.
    llvm::BasicBlock *BB =
        llvm::BasicBlock::Create(Context, "EntryBlock", Add1F);

    // Create a basic block builder with default parameters.  The builder
    // will automatically append instructions to the basic block `BB'.
    llvm::IRBuilder<> builder(BB);

    // Get pointers to the constant `1'.
    llvm::Value *One = builder.getInt32(1);

    // Get pointers to the integer argument of the add1 function...
    assert(Add1F->arg_begin() != Add1F->arg_end());  // Make sure there's an arg
    llvm::Argument *ArgX = &*Add1F->arg_begin();     // Get the arg
    ArgX->setName("AnArg");  // Give it a nice symbolic name for fun.

    // Create the add instruction, inserting it into the end of BB.
    llvm::Value *Add = builder.CreateAdd(One, ArgX);

    // Create the return instruction and add it to the basic block
    builder.CreateRet(Add);

    // Now, function add1 is ready.

    // Now we're going to create function `foo', which returns an int and
    // takes no arguments.
    llvm::Function *FooF = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(Context), {}, false),
        llvm::Function::ExternalLinkage, "foo", M);

    // Add a basic block to the FooF function.
    BB = llvm::BasicBlock::Create(Context, "EntryBlock", FooF);

    // Tell the basic block builder to attach itself to the new basic block
    builder.SetInsertPoint(BB);

    // Get pointer to the constant `10'.
    llvm::Value *Ten = builder.getInt32(10);

    // Pass Ten to the call to Add1F
    llvm::CallInst *Add1CallRes = builder.CreateCall(Add1F, Ten);
    Add1CallRes->setTailCall(true);

    // Create the return instruction and add it to the basic block.
    builder.CreateRet(Add1CallRes);

    // ---------------------------- JIT -----------------------------------

    // Now we create the JIT.
    llvm::ExecutionEngine *EE = llvm::EngineBuilder(std::move(Owner)).create();

    if (settings.showModule) {
        llvm::outs() << "We just constructed this LLVM module:\n\n" << *M;
        llvm::outs() << "\n\nRunning foo: ";
        llvm::outs().flush();
    }

    // Call the `foo' function with no arguments:
    std::vector<llvm::GenericValue> noargs;
    llvm::GenericValue gv = EE->runFunction(FooF, noargs);

    // Import result of execution:

    if (settings.showResult) {
        llvm::outs() << "Result: " << gv.IntVal << "\n";
    }

    delete EE;

    return Mlang::Signal::Success;
}

Mlang::Signal Mlang::executeFile(std::string thePath) {
    std::ifstream stream(thePath);
    std::stringstream strBuffer;
    strBuffer << stream.rdbuf();

    auto fileContent = strBuffer.str();

    if (settings.showFileContent) {
        std::cout << "File: " << thePath << std::endl
                  << fileContent << "<EOF>" << std::endl;
    }

    if (!fileContent.empty()) {
        return executeString(fileContent);
    } else {
        std::cout << "File empty: " << thePath << std::endl;
        return Mlang::Signal::Failure;
    }
}