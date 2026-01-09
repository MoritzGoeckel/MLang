#include <iostream>
#include <string>

#include "../core/Mlang.h"
#include "../error/Exceptions.h"
#include "../application/ArgumentsParser.h"

int main(int argc, char** argv) {
    ArgumentsParser args(argc, argv);
    if (!args.isSuccess()) {
        std::cerr << "Failed to parse arguments" << std::endl;
        return 1;
    }

    const auto& scriptFileOpt = args.getArgument(0);
    if (!scriptFileOpt) {
        std::cerr << "No script file provided" << std::endl;
        return 1;
    }

    std::string scriptFile = *scriptFileOpt;

    core::Mlang mlang;

    bool showAll = args.hasFlag("debug");

    mlang.settings.showTokens = args.hasFlag("show-tokens") || showAll;
    mlang.settings.showFileContent = args.hasFlag("show-file-content") || showAll;
    mlang.settings.showResult = args.hasFlag("show-result") || showAll;
    mlang.settings.showAbstractSyntaxTree = args.hasFlag("show-ast") || showAll;
    mlang.settings.showTypeInference = args.hasFlag("show-type-inference") || showAll;
    mlang.settings.showInferedTypes = args.hasFlag("show-inferred-types") || showAll;
    mlang.settings.showFunctions = args.hasFlag("show-functions") || showAll;
    mlang.settings.showEmission = args.hasFlag("show-emission") || showAll;
    mlang.settings.showExecution = args.hasFlag("show-execution") || showAll;
    mlang.settings.maxInstructions = 0; // 0 means no limit

    int exitCode = 0;
    try {
        auto rs = mlang.executeFile(scriptFile);
        if (rs == core::Mlang::Result::Signal::Success) {
            const auto& result = rs.getResult();

            if (result == "void"){
                exitCode = 0;
            } else {
                try {
                    exitCode = std::stoi(result);
                } catch (const std::invalid_argument&) {
                    // Non-integer result, just print it
                    std::cout << "Result: " << result << std::endl;
                    exitCode = 0;
                }
            }
        } else {
            std::cout << rs.getErrorString() << std::endl;
        }
    } catch (const MException& e) {
        std::cout << "Execution failed with exception: " << e.show(true)
                  << std::endl;
        exitCode = 1;
    }

    return exitCode;
}
