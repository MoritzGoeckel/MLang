SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
SET (CMAKE_CXX_FLAGS                "-std=c++11 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -lLLVM-9")

# SET (CMAKE_CXX_FLAGS                "-I/usr/lib/llvm-9/include -std=c++11 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-9/lib -lLLVM-9")

SET (CMAKE_AR      "/usr/bin/llvm-ar")
SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
SET (CMAKE_NM      "/usr/bin/llvm-nm")
SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# compiler must be 11 or 14
set(CMAKE_CXX_STANDARD 11)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)
# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE /home/moritz/.local/lib/antlr-4.7.1-complete.jar)
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
add_executable(tests
  src/tests/Tests.cpp
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
target_link_libraries(tests
    gtest pthread)

# Call macro to add lexer and grammar to your build dependencies.
antlr_target(MGrammar MGrammar.g4
             PACKAGE MGrammar
             LISTENER
             VISITOR
             OUTPUT_DIRECTORY gen)

# include generated files in project environment
include_directories(${ANTLR_MGrammar_OUTPUT_DIR}
                    /usr/lib/llvm-9/include)

# add generated grammar to demo binary target
add_executable(exec src/main.cpp
               ${ANTLR_MGrammar_CXX_OUTPUTS})
target_link_libraries(exec antlr4_static)

add_executable(execLlvm src/llvmMain.cpp
               ${ANTLR_MGrammar_CXX_OUTPUTS})
target_link_libraries(execLlvm antlr4_static) # /usr/lib/llvm-9/lib

# LLVM
# clang++ -I/usr/lib/llvm-9/include -std=c++11 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-9/lib -lLLVM-9 llvmMain.cpp
