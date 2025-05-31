#include "../executer/ByteCode.h"
#include <iostream>
#include "../core/Mlang.h"
#include <fstream>
#include <sstream>
#include <string>
#include <optional>

#define EXPECT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << " Expected: " << (expected) << ", but got: " << (actual) << std::endl; \
        throw std::runtime_error("Test failed!"); \
    }

#define EXPECT_TRUE(condition) \
    if (!(condition)) { \
        std::cerr << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << " Expected condition to be true, but it was false." << std::endl; \
        throw std::runtime_error("Test failed!"); \
    }

#define EXPECT_TRUE_PRINT(condition, output) \
    if (!(condition)) { \
        std::cerr << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << " Expected condition to be true, but it was false." << std::endl; \
        std::cerr << "Error: " << output << std::endl; \
        throw std::runtime_error("Test failed!"); \
    }

std::map<std::string, std::string> readMetadata(const std::string& path) {
    std::map<std::string, std::string> result;
    std::ifstream stream(path);
    std::string line;
    while (std::getline(stream, line)) {
        // trim whitespace
        auto trim = [&line]() {
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
        };
        trim();
        // check if the line starts with #
        if (line.empty() || line[0] != '#') {
            continue;
        }

        // remove the leading #
        line.erase(0, 1);

        // trim again
        trim();

        // split by =
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            result[key] = value;
        }
    }
    return result;
}

bool compareResults(const std::string& expected, const std::string& actual) {
    if (expected == actual) {
        return true;
    }
    if (expected == "true" && actual == "1") {
        return true;
    }
    if (expected == "false" && actual == "0") {
        return true;
    }
    return false;
}

void testFile(std::string path){
    std::cout << "Testing file: " << path << std::endl;

    auto metadata = readMetadata(path);
    std::optional<std::string> expectResult;
    auto expectIt = metadata.find("expect_result");
    if (expectIt != metadata.end()) {
        expectResult = expectIt->second;
    }

    std::optional<std::string> expect_failure;
    auto expect_failure_it = metadata.find("failure");
    if (expect_failure_it  != metadata.end()) {
        expect_failure = expect_failure_it  ->second;
    }

    core::Mlang mlang;
    mlang.settings.showTokens = true;
    mlang.settings.showFileContent = true;
    mlang.settings.showResult = true;
    mlang.settings.showAbastractSyntaxTree = true;
    mlang.settings.showInferedTypes = true;
    mlang.settings.showFunctions = true;
    mlang.settings.showEmission = true;

    auto rs = mlang.executeFile(path);

    if (rs == core::Mlang::Result::Signal::Failure){
        std::cout << "Error: " << rs.getErrorString() << std::endl;
    }

    EXPECT_TRUE(expectResult || expect_failure);

    if (expect_failure) {
        EXPECT_TRUE(rs == core::Mlang::Result::Signal::Failure);
        // EXPECT_EQ(expect_failure.value(), rs.getErrorString());
    } else if (expectResult) {
        EXPECT_TRUE(rs == core::Mlang::Result::Signal::Success);
        EXPECT_TRUE_PRINT(compareResults(expectResult.value(), rs.getResult()),
            expectResult.value() << " != " << rs.getResult());
    } else {
        EXPECT_TRUE(rs == core::Mlang::Result::Signal::Success);
    }

    std::cout << "[ OK ] " << path << std::endl;
}

int main() {
    testFile("mfiles/addition_infix.m");
    testFile("mfiles/addition.m");
    testFile("mfiles/broken_syntax.m");
    testFile("mfiles/broken_semantics.m");
    testFile("mfiles/comment.m");
    testFile("mfiles/if_else_no_brackets.m");
    testFile("mfiles/if_else.m");
    testFile("mfiles/if.m");
    testFile("mfiles/infix_operator_precedence.m");
    testFile("mfiles/method_decl_multiline.m");
    testFile("mfiles/method_declaration_brackets.m");
    testFile("mfiles/method_declaration.m");
    testFile("mfiles/multiple_vars.m");
    testFile("mfiles/simple_fns.m");
    testFile("mfiles/while.m");
    testFile("mfiles/return_in_some_cases_0.m");
    testFile("mfiles/return_in_some_cases_1.m");
    testFile("mfiles/ignored_return.m");
    testFile("mfiles/function_without_return.m");
    testFile("mfiles/var_declaration_addition.m");
    testFile("mfiles/var_declaration.m");

    // TODO: Structs
    testFile("mfiles/struct.m");

    // TODO: Type inference not working.
    // We should skip recursive calls and find the terminating return.
    // Assume that type for the recursive calls and see if this passes
    // without conflicts.
    // testFile("mfiles/recursion.m");

    // structs (going to the heap, accessors for fields)
    //    member functions for structs

    // blob type (alloc8, size, get(blob, idx), set(blob, idx))
    //    synatx sugar for get, set with []

    // Use blob and struct to implement:
    //   arrays
    //   strings
    //   maps?

    // Type annotations

    // floats
    // bools

    // c ffi, interop (dlopen / dlsym) and (LoadLibrary / GetProcAddress)
    //   terminal IO
    //   file IO

    return 0;
}
