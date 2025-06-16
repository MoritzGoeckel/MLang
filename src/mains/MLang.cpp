#include <iostream>
#include <string>

#include "../core/Mlang.h"
#include "../error/Exceptions.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Expected 1 argument but got " << std::to_string(argc - 1)
                  << std::endl;
        return 2;
    }

    std::string scriptFile(argv[1]);

    core::Mlang mlang;

    mlang.settings.showTokens = false;
    mlang.settings.showFileContent = false;
    mlang.settings.showResult = false;
    mlang.settings.showAbastractSyntaxTree = false;
    mlang.settings.showTypeInference = false;
    mlang.settings.showInferedTypes = false;
    mlang.settings.showFunctions = false;
    mlang.settings.showEmission = false;
    // TODO: Config params from cmd

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
