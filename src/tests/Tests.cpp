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
         "008_method_decl_multiline.m", "009_comment.m"});

    Mlang mlang;
    // mlang.settings.showParseTree = true;
    mlang.settings.showPrettyParseTree = true;
    // mlang.settings.showFileContent = true;

    for (auto& str : files) {
        mlang.executeFile("mfiles/" + str);
        std::cout << std::endl;
        // TODO: Error checking
    }
}

int main(int argc, char** argv) {
    Mlang::init();

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    Mlang::shutdown();
    return result;
}
