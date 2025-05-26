#include "../executer/ByteCode.h"
#include <iostream>
#include "../core/Mlang.h"

#define EXPECT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << " Expected: " << (expected) << ", but got: " << (actual) << std::endl; \
        throw std::runtime_error("Test failed!"); \
    }

void testFile(std::string path){
    std::cout << "Testing file: " << path << std::endl;

    core::Mlang mlang;
    mlang.settings.showTokens = false;
    mlang.settings.showFileContent = false;
    mlang.settings.showResult = false;
    mlang.settings.showAbastractSyntaxTree = false;
    mlang.settings.showInferedTypes = false;
    mlang.settings.showFunctions = true;
    mlang.settings.showEmission = true;

    auto rs = mlang.executeFile(path);
    if (rs == core::Mlang::Result::Signal::Success) {
        std::cout << "Result: " << rs.getResult() << std::endl;
    } else {
        std::cout << rs.getErrorString() << std::endl;
    }
}

int main() {
    testFile("mfiles/addition.m");
    testFile("mfiles/addition_infix.m");
    testFile("mfiles/comment.m");
    testFile("mfiles/if.m");
    testFile("mfiles/if_else_no_brackets.m");
    testFile("mfiles/if_else.m");
    return 0;
}
