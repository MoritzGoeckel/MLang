#include "Mlang.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "../executer/Runner.h"
#include "../parser/Parser.h"
#include "../parser/Tokenizer.h"
#include "../preprocessor/Preprocessor.h"
#include "../transformer/HasUnknownTypes.h"
#include "../transformer/ImplicitReturn.h"
#include "../transformer/CollectTypes.h"
#include "../transformer/UpdateOffsets.h"
#include "../transformer/InfereIdentifierTypes.h"
#include "../transformer/ApplyTypeAnnotations.h"
#include "../transformer/InfereParameterTypes.h"
#include "../transformer/InstantiateFunctions.h"
#include "../transformer/AddVoidReturn.h"
#include "../validator/AllPathsReturn.h"
#include "../emitter/Emitter.h"
#include "../emitter/Python.h"
#include "../emitter/ByteCodeEmitter.h"
#include "../executer/ByteCode.h"

namespace core {

Mlang::Mlang() {}

Mlang::~Mlang() {}

Mlang::Result Mlang::executeString(const std::string& theCode) {
    return execute("internal", theCode);
}

Mlang::Result Mlang::execute(const std::string& theFile,
                             const std::string& theCode) {
    Tokenizer tokenizer(theFile, theCode);

    std::cout << "Tokens:" << std::endl;
    auto tokens = tokenizer.getTokens();
    if (settings.showTokens) {
        for (auto token : tokens) {
            std::cout << token << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    Parser parser(std::move(tokens));
    auto ast = parser.getAst();

    if (!ast) {
        return Mlang::Result(Mlang::Result::Signal::Failure)
            //.addError("Parsing failed:")
            .addError(parser.getError(theCode));
    }

    if (settings.showAbstractSyntaxTree) {
        std::cout << "Abstract Syntax Tree:" << std::endl;
        std::cout << ast->toString() << std::endl << std::endl;
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

        // TODO: Once check that structs have unique names

        CollectTypes::TypesMap structs;
        CollectTypes collectTypesWalker{structs};
        ApplyTypeAnnotations applyTypeAnnotationsWalker{structs};
        while (true) {
            // Errors in previous iteration are maybe fine, 
            // if we find fitting custom types later
            applyTypeAnnotationsWalker.clearErrors();

            applyTypeAnnotationsWalker.process(ast);
            collectTypesWalker.process(ast);
            updateOffsets(structs);

            // Identifier types
            InfereIdentifierTypes identTypesWalker;
            identTypesWalker.process(ast);

            if(settings.showTypeInference) {
                std::cout << "After InfereIdentifierTypes: " << std::endl;
                std::cout << ast->toString() << std::endl << std::endl;
            }

            // Function types / params
            InfereParameterTypes paramTypesWalker;
            paramTypesWalker.process(ast);

            if(settings.showTypeInference) {
                std::cout << "After InfereParameterTypes: " << std::endl;
                std::cout << ast->toString() << std::endl << std::endl;
            }

            validator.process(ast);
            if (validator.hasTypeConflicts() ||
                validator.isAllTypesResolved() ||
                validator.getNumUnresolved() >= lastUnresolved) {
                // Conflicts occurred, all types are resolved or number of
                // resolved types did not increase -> Stop
                break;
            } else {
                lastUnresolved = validator.getNumUnresolved();
                if(settings.showTypeInference) {
                    std::cout << "After round of type inference we have " << lastUnresolved << " unresolved types" << std::endl << std::endl;
                }
                validator.reset();
            }
        }

        // Check for invalid type annotation errors after all iterations
        if (applyTypeAnnotationsWalker.hasErrors()) {
            const auto& annotationErrors = applyTypeAnnotationsWalker.getErrors();
            return Mlang::Result(Mlang::Result::Signal::Failure)
                .addError("Invalid type annotation:\n" +
                          annotationErrors.front().generateString(theCode));
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
            std::cout << ast->toString() << std::endl << std::endl;
        }
    }

    InstantiateFunctions instantiator(ast); // Do not use ast after this point!
    auto fns = instantiator.getFunctions();

    // Validate that all code paths return a value (before AddVoidReturn fixes it)
    {
        validator::AllPathsReturn returnValidator;
        for (auto& fn : fns) {
            returnValidator.validate(fn.second);

            // Check for errors after each function
            if (returnValidator.hasErrors()) {
                const auto& returnErrors = returnValidator.getErrors();
                return Mlang::Result(Mlang::Result::Signal::Failure)
                    .addError("Return path validation failed:\n" +
                              returnErrors.front().generateString(theCode));
            }
        }
    }

    for (auto& fn : fns) {
        transformer::AddVoidReturn addVoidReturn;
        fn.second = addVoidReturn.process(fn.second);
    }

    if (settings.showFunctions) {
        for (auto& fn : fns) {
            std::cout << "### " << fn.first << " ###" << std::endl;
            std::cout << fn.second->AST::Node::toString() << std::endl;
            std::cout << std::endl;
        }
    }

    // TODO: Make sure after a return no other statements exist, otherwise create error

    emitter::ByteCodeEmitter byteCodeEmitter(fns);
    byteCodeEmitter.run();

    if (settings.showEmission) {
        std::cout << "Bytecode:" << std::endl;
        std::cout << byteCodeEmitter.toString() << std::endl;
    }

    auto program = byteCodeEmitter.getProgram();
    executor::ByteCodeVM runner(program);
    runner.setDebug(settings.showExecution);
    auto result = runner.execute(settings.maxInstructions);

    return Mlang::Result(Mlang::Result::Signal::Success, result);
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

}
