#include <string>
#include <vector>

#include "../Mlang.h"
#include "../exceptions/Exceptions.h"
#include "gtest/gtest.h"

TEST(Mlang, ExecuteStringSimple) {
    Mlang mlang;
    mlang.executeString("let i = 10;");
}

TEST(Mlang, ExecuteSimple) {
    Mlang mlang;
    mlang.executeFile("mfiles/001_addition.m");
}

TEST(Mlang, ExecuteFiles) {
    std::vector<std::string> files(
        {"addition.m", "addition_infix.m", "var_declaration.m",
         "var_declaration_addition.m", "mutiple_vars.m", "method_declaration.m",
         "method_declaration_brackets.m", "method_decl_multiline.m",
         "comment.m", "if.m", "if_else.m", "while.m", "if_else_no_brackets.m",
         "simple_fns.m" /*, "recursion.m"*/});

    // TODO: recursion.m is still broken

    std::string basePath("");

    Mlang mlang;
    mlang.settings.showFileContent = true;
    mlang.settings.showFunctions = true;

    for (auto& str : files) {
        try {
            auto rs = mlang.executeFile(basePath + "mfiles/" + str);
            ASSERT_TRUE(rs == Mlang::Signal::Success);
            std::cout << std::endl;
        } catch (MException e) {
            std::cout << e.show(true) << std::endl;
            FAIL();
        }
    }
}

int main(int argc, char** argv) {
    Mlang::init();

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    Mlang::shutdown();
    return result;
}
