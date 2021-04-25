#include <iostream>
#include <string>

#include "Mlang.h"
#include "exceptions/Exceptions.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Expected 1 argument but got " << std::to_string(argc - 1)
                  << std::endl;
        return 2;
    }

    std::string scriptFile(argv[1]);

    std::cout << "Executing file: " << scriptFile << std::endl;

    Mlang::init();
    Mlang mlang;

    mlang.settings.showTokens = false;
    mlang.settings.showFileContent = false;
    mlang.settings.showResult = false;
    mlang.settings.showAbastractSyntaxTree = false;
    mlang.settings.showInferedTypes = false;
    mlang.settings.showFunctions = false;
    mlang.settings.showOptimizedModule = false;
    // TODO: Config params from cmd

    int exitCode = 0;
    try {
        auto rs = mlang.executeFile(scriptFile);
        if (rs == Mlang::Result::Signal::Success) {
            std::cout << "Result: " << rs.getResult() << std::endl;
            std::cout << "Execution succeeded!" << std::endl;
        } else {
            std::cerr << "Errors: " << rs.getErrorString() << std::endl;
            std::cerr << "Execution failed!" << std::endl;
        }
    } catch (MException e) {
        std::cerr << "Execution failed with exception: " << e.show(true)
                  << std::endl;
        exitCode = 1;
    }

    Mlang::shutdown();
    return exitCode;
}
