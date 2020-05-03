#include <string>
#include <vector>

#include "../Mlang.h"
#include "gtest/gtest.h"

TEST(Mlang, ExecuteStringSimple) {
    Mlang mlang;
    mlang.executeString("let i = 10;");
    // EXPECT_EQ(15, 1);
}

TEST(Mlang, ExecuteSimple) {
    Mlang mlang;
    mlang.executeFile("mfiles/001_addition.m");
}

TEST(Mlang, ExecuteFiles) {
    std::vector<std::string> files(
        {"001_addition.m", "002_addition_infix.m", "003_var_declaration.m",
         "004_var_declaration_addition.m", "005_mutiple_vars.m",
         "006_method_declaration.m", "007_method_declaration_brackets.m",
         "008_method_decl_multiline.m", "009_comment.m", "010_if.m",
         "011_if_else.m", "012_while.m", "013_if_else_no_brackets.m",
         "014_simple_fns.m" /*, "015_recursion.m"*/});

    // TODO: 015_recursion.m is still broken

    Mlang mlang;
    mlang.settings.showFileContent = true;
    mlang.settings.showFunctions = true;

    for (auto& str : files) {
        auto rs = mlang.executeFile("mfiles/" + str);
        EXPECT_TRUE(rs == Mlang::Signal::Success);
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    Mlang::init();

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    Mlang::shutdown();
    return result;
}
