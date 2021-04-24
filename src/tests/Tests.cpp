#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "../Mlang.h"
#include "../exceptions/Exceptions.h"
#include "../parser/Parser.h"
#include "../parser/Tokenizer.h"
#include "gtest/gtest.h"

class MLangTest : public ::testing::Test {
   public:
    MLangTest() : basePath(""), mlang() {}

    std::vector<std::string> getFileContents() {
        std::vector<std::string> contents;

        for (auto filename : getFullFilenames()) {
            std::ifstream stream(filename);
            std::stringstream strBuffer;
            strBuffer << stream.rdbuf();
            contents.push_back(strBuffer.str());
        }
        return contents;
    }

    std::vector<std::string> getFullFilenames() {
        std::vector<std::string> output;
        for (auto& filename : getFilenames()) {
            output.push_back(basePath + "mfiles/" + filename);
        }
        return output;
    }

    std::vector<std::string> getFilenames() {
        return {"addition.m",
                "addition_infix.m",
                "var_declaration.m",
                "var_declaration_addition.m",
                "mutiple_vars.m",
                "method_declaration.m",
                "method_declaration_brackets.m",
                "method_decl_multiline.m",
                "comment.m",
                "if.m",
                "if_else.m",
                "while.m",
                "if_else_no_brackets.m",
                "simple_fns.m"};
    }

   protected:
    void SetUp() override {
        mlang.settings.showFileContent = false;
        mlang.settings.showFunctions = false;
        mlang.settings.showTokens = false;
        mlang.settings.showInferedTypes = false;
        mlang.settings.showAbastractSyntaxTree = false;
    }

   private:
    std::string basePath;

   public:
    Mlang mlang;
};

TEST_F(MLangTest, ExecuteStringSimple) { mlang.executeString("let i = 10;"); }

TEST_F(MLangTest, ExecuteSimple) { mlang.executeFile("mfiles/addition.m"); }

TEST_F(MLangTest, Tokenizer) {
    const bool show = false;

    for (auto& fileContent : getFileContents()) {
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

TEST_F(MLangTest, Parser) {
    const bool show = false;

    for (auto& fileContent : getFileContents()) {
        Tokenizer tokenizer(fileContent);
        auto tokens = tokenizer.getTokens();

        Parser parser(tokens);
        auto rootNode = parser.getAst();

        if (!rootNode || show) {
            std::cout << fileContent << std::endl;
            for (auto& t : tokens) {
                std::cout << " " << t;
            }
            std::cout << std::endl;
            std::cout << "Parse " << (rootNode ? "succeeded" : "failed!")
                      << std::endl;
        }
        ASSERT_TRUE(rootNode);

        if constexpr (show) {
            std::cout << rootNode->toString() << std::endl;
            std::cout << "------------------------------------" << std::endl;
        }
    }

    // TODO: Have some baseline to compare to
}

TEST_F(MLangTest, ExecuteFiles) {
    // "recursion.m" TODO: recursion.m is still broken

    for (auto& filename : getFullFilenames()) {
        try {
            auto rs = mlang.executeFile(filename);
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
