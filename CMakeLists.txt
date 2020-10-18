# Some notes:
# SET (CMAKE_CXX_FLAGS    "-I/usr/lib/llvm-9/include -std=c++11 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-9/lib -lLLVM-9")
# clang++ -I/usr/lib/llvm-9/include -std=c++11 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-9/lib -lLLVM-9 llvmMain.cpp
# /usr/lib/llvm-9/lib

project(MLang)

#SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++")
#SET (CMAKE_CXX_FLAGS                "-std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -lLLVM-10")
SET (CMAKE_CXX_FLAGS                "-D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS /EHa") # /EHa /EHs allows exceptions

# --libs --ldflags
# -LIBPATH:D:\Software\llvm\llvm\bin\Release\lib

set(CMAKE_CXX_STANDARD 17)

#SET (CMAKE_AR      "/usr/bin/llvm-ar")
#SET (CMAKE_LINKER  "/usr/bin/llvm-ld")
#SET (CMAKE_NM      "/usr/bin/llvm-nm")
#SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump")
#SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 17)

# ANTLR

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

include(ExternalAntlr4Cpp)

include_directories(${ANTLR4_INCLUDE_DIRS})

# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
#set(ANTLR_EXECUTABLE /usr/local/bin/antlr-4.8-complete.jar) # TODO! on windows
set(ANTLR_EXECUTABLE D:/Software/java-libs/antlr-4.8-complete.jar)

find_package(ANTLR REQUIRED)

antlr_target(MGrammar MGrammar.g4
             PACKAGE MGrammar
             LISTENER
             VISITOR
             OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gen)

include_directories(${ANTLR_MGrammar_OUTPUT_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

## Gtest

#include_directories(${GTEST_INCLUDE_DIRS})
#include_directories("D:/Software/googletest-release-1.10.0/googletest/include")
include_directories("D:/Software/llvm/llvm/utils/unittest/googletest/include")

#set(GTEST_LIBRARY "D:/Software/googletest-release-1.10.0/googletest/lib/Release/gtest.lib")
#set(GTEST_MAIN_LIBRARY "D:/Software/googletest-release-1.10.0/googletest/lib/Release/gtest_main.lib")

set(GTEST_LIBRARY "D:/Software/llvm/llvm/bin/Release/lib/gtest.lib")
set(GTEST_MAIN_LIBRARY "D:/Software/llvm/llvm/bin/Release/lib/gtest_main.lib")

#set(GTEST_INCLUDE_DIR "D:/Software/googletest-release-1.10.0/googletest/include")
set(GTEST_INCLUDE_DIR "D:/Software/llvm/llvm/utils/unittest/googletest/include")

#link_directories("D:/Software/googletest-release-1.10.0/googletest/lib/Release/")

find_package(GTest REQUIRED)

## LLVM

include_directories("D:/Software/llvm/llvm/include")
include_directories("D:/Software/llvm/llvm/bin/include")
#include_directories("/usr/lib/llvm-10/include")

link_directories("D:/Software/llvm/llvm/bin/Release/lib")

# Defining executables

add_executable(tests
  src/tests/Tests.cpp
  src/Mlang.cpp
  src/executer/LLVMRunner.cpp
  src/parser/PrintVisitor.cpp
  src/parser/PtToAstVisitor.cpp
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
  ${ANTLR_MGrammar_CXX_OUTPUTS}
)
target_link_libraries(tests gtest gtest_main antlr4_static
    LLVMXRay LLVMWindowsManifest LLVMTestingSupport LLVMTableGen LLVMSymbolize LLVMDebugInfoPDB LLVMOrcJIT LLVMOrcError LLVMJITLink LLVMObjectYAML LLVMMCA LLVMLTO LLVMPasses LLVMCoroutines LLVMObjCARCOpts LLVMExtensions LLVMLineEditor LLVMLibDriver LLVMInterpreter LLVMInterfaceStub LLVMFuzzMutate LLVMMCJIT LLVMExecutionEngine LLVMRuntimeDyld LLVMDWARFLinker LLVMDlltoolDriver LLVMOption LLVMDebugInfoGSYM LLVMCoverage LLVMXCoreDisassembler LLVMXCoreCodeGen LLVMXCoreDesc LLVMXCoreInfo LLVMX86Disassembler LLVMX86AsmParser LLVMX86CodeGen LLVMX86Desc LLVMX86Info LLVMWebAssemblyDisassembler LLVMWebAssemblyCodeGen LLVMWebAssemblyDesc LLVMWebAssemblyAsmParser LLVMWebAssemblyInfo LLVMSystemZDisassembler LLVMSystemZCodeGen LLVMSystemZAsmParser LLVMSystemZDesc LLVMSystemZInfo LLVMSparcDisassembler LLVMSparcCodeGen LLVMSparcAsmParser LLVMSparcDesc LLVMSparcInfo LLVMRISCVDisassembler LLVMRISCVCodeGen LLVMRISCVAsmParser LLVMRISCVDesc LLVMRISCVUtils LLVMRISCVInfo LLVMPowerPCDisassembler LLVMPowerPCCodeGen LLVMPowerPCAsmParser LLVMPowerPCDesc LLVMPowerPCInfo LLVMNVPTXCodeGen LLVMNVPTXDesc LLVMNVPTXInfo LLVMMSP430Disassembler LLVMMSP430CodeGen LLVMMSP430AsmParser LLVMMSP430Desc LLVMMSP430Info LLVMMipsDisassembler LLVMMipsCodeGen LLVMMipsAsmParser LLVMMipsDesc LLVMMipsInfo LLVMLanaiDisassembler LLVMLanaiCodeGen LLVMLanaiAsmParser LLVMLanaiDesc LLVMLanaiInfo LLVMHexagonDisassembler LLVMHexagonCodeGen LLVMHexagonAsmParser LLVMHexagonDesc LLVMHexagonInfo LLVMBPFDisassembler LLVMBPFCodeGen LLVMBPFAsmParser LLVMBPFDesc LLVMBPFInfo LLVMAVRDisassembler LLVMAVRCodeGen LLVMAVRAsmParser LLVMAVRDesc LLVMAVRInfo LLVMARMDisassembler LLVMARMCodeGen LLVMARMAsmParser LLVMARMDesc LLVMARMUtils LLVMARMInfo LLVMAMDGPUDisassembler LLVMAMDGPUCodeGen LLVMMIRParser LLVMipo LLVMInstrumentation LLVMVectorize LLVMLinker LLVMIRReader LLVMAsmParser LLVMFrontendOpenMP LLVMAMDGPUAsmParser LLVMAMDGPUDesc LLVMAMDGPUUtils LLVMAMDGPUInfo LLVMAArch64Disassembler LLVMMCDisassembler LLVMAArch64CodeGen LLVMCFGuard LLVMGlobalISel LLVMSelectionDAG LLVMAsmPrinter LLVMDebugInfoDWARF LLVMCodeGen LLVMTarget LLVMScalarOpts LLVMInstCombine LLVMAggressiveInstCombine LLVMTransformUtils LLVMBitWriter LLVMAnalysis LLVMProfileData LLVMObject LLVMTextAPI LLVMBitReader LLVMCore LLVMRemarks LLVMBitstreamReader LLVMAArch64AsmParser LLVMMCParser LLVMAArch64Desc LLVMMC LLVMDebugInfoCodeView LLVMDebugInfoMSF LLVMBinaryFormat LLVMAArch64Utils LLVMAArch64Info LLVMSupport LLVMDemangle)

# gtest_main

#target_link_libraries(tests
#    gtest antlr4_static) #pthread

#gtest_main gtest

add_executable(exec src/main.cpp
               ${ANTLR_MGrammar_CXX_OUTPUTS})
target_link_libraries(exec antlr4_static)

add_executable(execLlvm src/llvmMain.cpp
               ${ANTLR_MGrammar_CXX_OUTPUTS})
target_link_libraries(execLlvm antlr4_static)
