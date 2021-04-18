#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../Mlang.h"
#include "../exceptions/Exceptions.h"
#include "../parser/Parser.h"
#include "../parser/Tokenizer.h"
#include "gtest/gtest.h"

TEST(Mlang, ExecuteStringSimple) {
    Mlang mlang;
    mlang.executeString("let i = 10;");
}

TEST(Mlang, ExecuteSimple) {
    Mlang mlang;
    mlang.executeFile("mfiles/addition.m");
}

TEST(Mlang, Tokenizer) {
    std::vector<std::string> files(
        {"addition.m", "addition_infix.m", "var_declaration.m",
         "var_declaration_addition.m", "mutiple_vars.m", "method_declaration.m",
         "method_declaration_brackets.m", "method_decl_multiline.m",
         "comment.m", "if.m", "if_else.m", "while.m", "if_else_no_brackets.m",
         "simple_fns.m"});

    std::string basePath("");

    const bool show = false;

    for (auto& str : files) {
        std::string path = basePath + "mfiles/" + str;

        std::ifstream stream(path);
        std::stringstream strBuffer;
        strBuffer << stream.rdbuf();
        auto fileContent = strBuffer.str();

        Tokenizer tokenizer(fileContent);

        if constexpr (show) {
            std::cout << fileContent << std::endl;
            for (auto& t : tokenizer.getTokens()) {
                std::cout << " " << t;
            }
            std::cout << std::endl;
            std::cout << "---------------" << std::endl;
        }
    }

    // TODO: Have some baseline to compare to
}

TEST(Mlang, Parser) {
    std::vector<std::string> files(
        {"addition.m", "addition_infix.m", "var_declaration.m",
         "var_declaration_addition.m", "mutiple_vars.m", "method_declaration.m",
         "method_declaration_brackets.m", "method_decl_multiline.m",
         "comment.m", "if.m", "if_else.m", "while.m", "if_else_no_brackets.m",
         "simple_fns.m"});

    std::string basePath("");

    for (auto& str : files) {
        std::string path = basePath + "mfiles/" + str;

        std::ifstream stream(path);
        std::stringstream strBuffer;
        strBuffer << stream.rdbuf();
        auto fileContent = strBuffer.str();

        Tokenizer tokenizer(fileContent);
        auto tokens = tokenizer.getTokens();

        Parser parser(tokens);
        auto rootNode = parser.getAst();

        if (!rootNode) {
            std::cout << fileContent << std::endl;
            for (auto& t : tokens) {
                std::cout << " " << t;
            }
            std::cout << std::endl;
            std::cout << "Parse failed!" << std::endl;
        }
        ASSERT_TRUE(rootNode);
    }

    // TODO: Have some baseline to compare to
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
    mlang.settings.showFileContent = false;
    mlang.settings.showFunctions = false;

    for (auto& str : files) {
        try {
            auto rs = mlang.executeFile(basePath + "mfiles/" + str);
            ASSERT_TRUE(rs == Mlang::Signal::Success);
            if (mlang.settings.showFileContent || mlang.settings.showFunctions)
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
