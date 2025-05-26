#pragma once

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>

#include "../error/Exceptions.h"

namespace executor {

using word_t = unsigned long;

enum class Op { LOCALS, LOCALL, CALL, RET, PUSH, POP, WRITE_STACK, ADD, SUB, MUL, DIV, MOD, JUMP, JUMP_IF, ALLOC, WRITE_HEAP, READ_HEAP, PRINTS, TERM };

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

struct Program {
    std::vector<word_t> data; // TODO: Unused
    std::vector<Instruction> code;
};

class ByteCodeVM {
    private:
        std::vector<word_t> stack;
        std::vector<word_t> heap;
        Program program;
        bool debug;

    bool run();

    public:
    ByteCodeVM(const Program& program);
    std::string execute();

};

}