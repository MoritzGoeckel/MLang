#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>

#include "../executer/ExternalFunctions.h"
#include "../error/Exceptions.h"
#include "Types.h"
#include "Stack.h"

namespace executor {

enum class Op { 
    NOP, 
    LOCALS, 
    LOCALL, 
    CALL, 
    RET, 
    PUSH, 
    POP, 
    ADD, 
    SUB, 
    MUL, 
    DIV, 
    MOD, 
    JUMP, 
    JUMP_IF, 
    ALLOC, 
    PRINTS, 
    TERM, 
    LT, 
    GT, 
    EQ, 
    LTE, 
    GTE, 
    NEQ, 
    LOADW, 
    STOREW, 
    DUB,
    REG_FFI, // TODO: Rename FFI_...
    PUSH_FFI_WORD, 
    PUSH_FFI_DWORD, 
    PUSH_FFI_QWORD, 
    PUSH_FFI_XWORD, 
    CALL_FFI,
    DATA_ADDR
};

// TODO: ADD etc should be type specific, so IADD, FADD
// TODO: Logic operator missing: AND, OR, NOT

struct Instruction {
    Instruction(Op op, word_t arg1 = 0, word_t arg2 = 0, word_t arg3 = 0);

    Op op;
    word_t arg1;
    word_t arg2;
    word_t arg3;
};

struct OpCodeMetadata {
    std::string name;
    std::vector<std::string> arg_names;
};

std::string instructionsToString(const std::vector<Instruction>& instructions, bool named_args = false);

class Data {
    // TODO: Bring into cpp
    private:
    using byte_t = unsigned char;
    static_assert(sizeof(byte_t) == 1);
    std::vector<byte_t> data;

    public:
    Data(): data() {}

    char* getString(size_t idx){
        if (idx >= data.size()) {
            throwConstraintViolated("Data: Index out of bounds");
        }
        return reinterpret_cast<char*>(&data[idx]);
    }

    size_t addString(const std::string& str) {
        size_t startIdx = data.size();
        data.resize(startIdx + str.size() + 1); // +1 for null terminator

        for (size_t i = 0; i < str.size(); ++i) {
            data[startIdx + i] = static_cast<byte_t>(str[i]);
        }
        data[startIdx + str.size()] = 0; // Null terminator
        return startIdx;
    }

    void* getAddr(size_t idx) {
        if (idx >= data.size()) {
            throwConstraintViolated("Data: Index out of bounds");
        }
        return reinterpret_cast<void*>(&data[idx]);
    }
};

struct Program {
    Data data;
    std::vector<Instruction> code;
};

enum class ProgramState {
    Paused,
    Finished
};

class ByteCodeVM {
    private:
        word_t idx;

        // Registers to manage stack-based function calls, parameters, locals, and return values
        word_t function_stack_base;
        word_t return_value;

        Stack stack;
        std::vector<word_t> heap;
        Program program;
        bool debug;
        ffi::ExternalFunctions ffiFunctions;
        ffi::Arguments ffiArgs;

    ProgramState run(size_t maxInstructions);

    public:
    ByteCodeVM(const Program& program);
    void setDebug(bool debug) { this->debug = debug; }
    std::string execute(size_t maxInstructions);

};

}
