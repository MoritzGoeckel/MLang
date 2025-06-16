#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>

#include "../executer/ExternalFunctions.h"
#include "../error/Exceptions.h"
#include "Stack.h"

namespace executor {

using word_t = unsigned long;
static_assert(sizeof(word_t) == 8);

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
    REG_FFI,
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

// TODO: We don't need any of this, we can just use the stack
// https://stackoverflow.com/questions/1395591/what-exactly-is-the-base-pointer-and-stack-pointer-to-what-do-they-point
// https://www.cs.virginia.edu/~evans/cs216/guides/x86.html

// We just need to introduce registers
// esp: Current stack top
// ebp: Stack base pointer, is set to esp in the beginning of a function
// eax: Return value of the function

// On function call:
// - Push the parameters in reverse order
// - Push return address (done by the CALL instruction)
// - Push the current ebp
// - Set ebp to esp
// - Push locals
// Within fucnction:
// - Use ebp + n to access parameters
// - Use ebp + n to access locals
// On function return:
// - Put result into eax register
// - Pop locals from the stack
// - Pop ebp from the stack, put into ebp register
// - Pop return address from the stack, and jump (done by the RET instruction)
// - Pop parameters from the stack (done by the caller, not the callee)
// - Get result from eax register

struct StackFrame {
    std::vector<word_t> locals;
    word_t return_address;
};

class ByteCodeVM {
    private:
        word_t idx;
        std::vector<StackFrame> callstack;
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
    std::string execute(size_t maxInstructions = 1000);

};

}
