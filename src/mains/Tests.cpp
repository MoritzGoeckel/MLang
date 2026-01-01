#ifdef SINGLE_HEADER
    #include "../../include/libmlang.h"
#else
    #include "../executer/ExternalFunctions.h"
    #include "../executer/ByteCode.h"
    #include "../core/Mlang.h"
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>

#define RUN_TEST_LABEL() \
    std::cout << "[ START ] " << __FUNCTION__ << std::endl;

#define END_TEST_LABEL() \
    std::cout << "[ OK    ] " << __FUNCTION__ << std::endl;

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

#define EXPECT_FALSE(condition) \
    if ((condition)) { \
        std::cerr << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << " Expected condition to be false, but it was true." << std::endl; \
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
    std::cout << "[ START ] " << path << std::endl;

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
    mlang.settings.showExecution = true;
    mlang.settings.showTypeInference = true;
    mlang.settings.maxInstructions = 1000;

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

    std::cout << "[ OK    ] " << path << std::endl;
}

void testLibrary(){
    RUN_TEST_LABEL();

    ffi::ExternalFunctions externalFunctions;

    auto test_ii_i = externalFunctions.add("test", "test_ii_i", ffi::ret_type::Number);
    {
        ffi::Arguments args;
        args.addDWord(5);
        args.addDWord(10);
        auto r = externalFunctions.call(test_ii_i, args);
        EXPECT_EQ(15, r);
    }

    auto test_iii_i = externalFunctions.add("test", "test_iii_i", ffi::ret_type::Number);
    {
        ffi::Arguments args;
        args.addDWord(5);
        args.addDWord(10);
        args.addDWord(15);
        auto r = externalFunctions.call(test_iii_i, args);
        EXPECT_EQ(30, r);
    }

    auto test_iiii_i = externalFunctions.add("test", "test_iiii_i", ffi::ret_type::Number);
    {
        ffi::Arguments args;
        args.addDWord(5);
        args.addDWord(10);
        args.addDWord(15);
        args.addDWord(20);
        auto r = externalFunctions.call(test_iiii_i, args);
        EXPECT_EQ(50, r);
    }

    std::cout << "Testing with 5 arguments..." << std::endl;

    auto test_iiiii_i = externalFunctions.add("test", "test_iiiii_i", ffi::ret_type::Number);
    {
        ffi::Arguments args;
        args.addDWord(5);
        args.addDWord(10);
        args.addDWord(15);
        args.addDWord(20);
        args.addDWord(25);
        auto r = externalFunctions.call(test_iiiii_i, args);
        EXPECT_EQ(75, r);
    }

    std::cout << "Testing with 6 arguments..." << std::endl;

    auto test_iiiiii_i = externalFunctions.add("test", "test_iiiiii_i", ffi::ret_type::Number);
    {
        ffi::Arguments args;
        args.addDWord(5);
        args.addDWord(10);
        args.addDWord(15);
        args.addDWord(20);
        args.addDWord(25);
        args.addDWord(30);
        auto r = externalFunctions.call(test_iiiiii_i, args);
        EXPECT_EQ(105, r);
    }

    std::cout << "Testing return boolean values..." << std::endl;

    auto test_ii_b = externalFunctions.add("test", "test_ii_b", ffi::ret_type::Bool);
    {
        ffi::Arguments args;
        args.addDWord(5);
        args.addDWord(5);
        auto r = externalFunctions.call(test_ii_b, args);
        EXPECT_EQ(1, r); // true
    }

    {
        ffi::Arguments args;
        args.addDWord(5);
        args.addDWord(6);
        auto r = externalFunctions.call(test_ii_b, args);
        EXPECT_EQ(0, r); // false
    }

    auto test_bb_b = externalFunctions.add("test", "test_bb_b", ffi::ret_type::Bool);
    {
        ffi::Arguments args;
        args.addWord(1); // true
        args.addWord(0); // false
        bool r = externalFunctions.call(test_bb_b, args);
        EXPECT_EQ(0, r); // false
    }

    auto test_pp_p = externalFunctions.add("test", "test_pp_p", ffi::ret_type::Ptr);
    {
        ffi::Arguments args;
        std::string a = "Hello";
        std::string b = "World";
        args.addQWord(reinterpret_cast<ffi::qword_t>(const_cast<char*>(a.c_str())));
        args.addQWord(reinterpret_cast<ffi::qword_t>(const_cast<char*>(b.c_str())));
        auto r = reinterpret_cast<char*>(externalFunctions.call(test_pp_p, args));
        // compare strings
        EXPECT_TRUE(r != nullptr);
        std::string result(r);
        std::string expected = "Hello from C!";
        EXPECT_EQ(expected, result);
    }

    END_TEST_LABEL();
}

void testExecutorData(){
    RUN_TEST_LABEL();
    executor::Data data;
    auto idx = data.addString("Hello");
    EXPECT_EQ("Hello", std::string(data.getString(idx)));

    idx = data.addString("World");
    EXPECT_EQ("World", std::string(data.getString(idx)));

    idx = data.addString("Hello World");
    EXPECT_EQ("Hello World", std::string(data.getString(idx)));

    // Check that the strings are null-terminated
    EXPECT_EQ('\0', data.getString(idx)[11]);
    END_TEST_LABEL();
}

void suiteTestfiles(){
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
    testFile("mfiles/type_annotation.m");
    testFile("mfiles/struct.m");
    testFile("mfiles/struct_nested.m");
    testFile("mfiles/extern.m");
    testFile("mfiles/str.m");

    // TODO: blob type (alloc8(size), get(blob, idx), set(blob, idx)):
    //    synatx sugar for get, set with []
    // testFile("mfiles/blob.m");

    // TODO: Type inference not working for recursive functions
    // We should skip recursive calls and find the terminating return.
    // Assume that type for the recursive calls and see if this passes
    // without conflicts.
    // testFile("mfiles/recursion.m");
}

int main() {
    suiteTestfiles();
    testLibrary();
    testExecutorData();

    return 0;
}
