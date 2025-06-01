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
#include "../transformer/InfereIdentifierTypes.h"
#include "../transformer/ApplyTypeAnnotations.h"
#include "../transformer/InfereParameterTypes.h"
#include "../transformer/InstantiateFunctions.h"
#include "../transformer/AddVoidReturn.h"
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

        ApplyTypeAnnotations applyTypeAnnotationsWalker;
        applyTypeAnnotationsWalker.process(ast);

        if(settings.showTypeInference) {
            std::cout << "After ApplyTypeAnnotation: " << std::endl;
            std::cout << ast->toString() << std::endl;
        }

        while (true) {
            // Identifier types
            InfereIdentifierTypes identTypesWalker;
            identTypesWalker.process(ast);

            if(settings.showTypeInference) {
                std::cout << "After InfereIdentifierTypes: " << std::endl;
                std::cout << ast->toString() << std::endl;
            }

            // Function types / params
            InfereParameterTypes paramTypesWalker;
            paramTypesWalker.process(ast);

            if(settings.showTypeInference) {
                std::cout << "After InfereParameterTypes: " << std::endl;
                std::cout << ast->toString() << std::endl;
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

    InstantiateFunctions instantiator(ast);

    // Do not use ast after this point!
    auto fns = instantiator.getFunctions();

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

    // TODO: If function returns nothing, add a void return as last statement of the body
    // Make sure after a return no other statements exist, otherwise create error
    // If a function has a return type, make sure that all paths return a value of that type

    emitter::ByteCodeEmitter byteCodeEmitter(fns);
    byteCodeEmitter.run();

    if (settings.showEmission) {
        std::cout << "Bytecode:" << std::endl;
        std::cout << byteCodeEmitter.toString() << std::endl;
    }

    auto program = byteCodeEmitter.getProgram();
    executor::ByteCodeVM runner(program);
    runner.setDebug(settings.showExecution);
    auto result = runner.execute();

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
