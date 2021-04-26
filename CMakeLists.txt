# Some notes:
# SET (CMAKE_CXX_FLAGS    "-I/usr/lib/llvm-9/include -std=c++11 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-9/lib -lLLVM-9")
# clang++ -I/usr/lib/llvm-9/include -std=c++11 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-9/lib -lLLVM-9 llvmMain.cpp
# /usr/lib/llvm-9/lib

SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
SET (CMAKE_CXX_FLAGS                "-std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -lLLVM-10")

SET (CMAKE_AR      "/usr/bin/llvm-ar")
SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
SET (CMAKE_NM      "/usr/bin/llvm-nm")
SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 17)

include_directories(${GTEST_INCLUDE_DIRS})

find_package(GTest REQUIRED)

include_directories(/usr/lib/llvm-10/include)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Custom targets

add_custom_target(loc
    COMMAND cloc src
)

# Defining executables

add_executable(Tests
  src/tests/Tests.cpp
  src/Mlang.cpp
  src/executer/LLVMRunner.cpp
  src/transformer/LLVMEmitter.cpp
  src/transformer/HasUnknownTypes.cpp
  src/transformer/ImplicitReturn.cpp
  src/transformer/InfereIdentifierTypes.cpp
  src/transformer/InfereParameterTypes.cpp
  src/transformer/InstantiateFunctions.cpp
  src/transformer/TreeWalker.cpp
  src/preprocessor/Preprocessor.cpp
  src/exceptions/Exceptions.cpp
  src/ast/DataType.cpp
  src/parser/OperatorPrecedence.cpp
  src/parser/Parser.cpp
  src/parser/Tokenizer.cpp
  src/parser/ParseError.cpp
  src/parser/SourcePosition.cpp
)
target_link_libraries(Tests
    gtest pthread)

add_executable(ExecuteFile
  src/ExecuteFile.cpp
  src/Mlang.cpp
  src/executer/LLVMRunner.cpp
  src/transformer/LLVMEmitter.cpp
  src/transformer/HasUnknownTypes.cpp
  src/transformer/ImplicitReturn.cpp
  src/transformer/InfereIdentifierTypes.cpp
  src/transformer/InfereParameterTypes.cpp
  src/transformer/InstantiateFunctions.cpp
  src/transformer/TreeWalker.cpp
  src/preprocessor/Preprocessor.cpp
  src/exceptions/Exceptions.cpp
  src/ast/DataType.cpp
  src/parser/OperatorPrecedence.cpp
  src/parser/Parser.cpp
  src/parser/Tokenizer.cpp
  src/parser/ParseError.cpp
  src/parser/SourcePosition.cpp
)
target_link_libraries(ExecuteFile
    gtest pthread)
