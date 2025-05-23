#pragma once

#include <iostream>
#include <list>
#include <vector>

#include "../error/Exceptions.h"

using word_t = unsigned long;

enum class Op { PUSH_STACK, POP_STACK, WRITE_STACK, ADD, SUB, MUL, DIV, MOD, JUMP, JUMP_IF, ALLOC, WRITE_HEAP, READ_HEAP, TERM };

struct Instruction {
    Instruction(Op op, word_t arg1 = 0, word_t arg2 = 0, word_t arg3 = 0)
        : op(op), arg1(arg1), arg2(arg2), arg3(arg3) {}

    Op op;
    word_t arg1;
    word_t arg2;
    word_t arg3;
};

struct Program {
    word_t* data; // TODO: Unused
    std::vector<Instruction> code;
};

class ByteCodeVM {
    private:
        std::vector<std::vector<word_t>> stacks;
        std::vector<word_t> heap;
        Program program;

    void run() {
        stacks.emplace_back(1);

        word_t idx = 0;
        while(true){
            const Instruction& inst = program.code[idx++];
            auto& stack = stacks.back();
            switch(inst.op){
                case Op::PUSH_STACK: {
                    // PUSH_STACK SIZE
                    stacks.emplace_back(inst.arg1);
                    break;
                }
                case Op::POP_STACK: {
                    // POP_STACK RET_STACK_ADDR
                    word_t ret = stack.back();
                    stacks.pop_back();
                    stacks.back().at(inst.arg1) = ret;
                    break;
                }
                case Op::WRITE_STACK: {
                    // WRITE_STACK STACK_ADDR VALUE
                    stack[inst.arg1] = inst.arg2;
                    break;
                }
                case Op::ADD: {
                    // ADD RESULT_ADDR DIVIDEND_ADDR DIVISOR_ADDR
                    stack[inst.arg1] = stack[inst.arg2] + stack[inst.arg3];
                    break;
                }
                case Op::SUB: {
                    stack[inst.arg1] = stack[inst.arg2] - stack[inst.arg3];
                    break;
                }
                case Op::MUL: {
                    stack[inst.arg1] = stack[inst.arg2] * stack[inst.arg3];
                    break;
                }
                case Op::DIV: {
                    stack[inst.arg1] = stack[inst.arg2] / stack[inst.arg3];
                    break;
                }
                case Op::MOD: {
                    stack[inst.arg1] = stack[inst.arg2] % stack[inst.arg3];
                    break;
                }
                case Op::JUMP: {
                    // JUMP ADDR
                    idx = inst.arg1;
                    break;
                }
                case Op::JUMP_IF: {
                    // JUMP_IF CONDITION_STACK_ADDR NOT_NULL_ADDR NULL_ADDR
                    if (stack[inst.arg1] != 0) {
                        idx = inst.arg2;
                    } else {
                        idx = inst.arg3;
                    }
                    break;
                }
                case Op::ALLOC: {
                    // ALLOC STACK_ADDR SIZE
                    stack[inst.arg1] = heap.size();
                    heap.resize(heap.size() + inst.arg2);
                    break;
                }
                case Op::WRITE_HEAP: {
                    // WRITE_HEAP FROM_STACK_ADDR TO_HEAP_ADDR SIZE
                    for (int i = 0; i < inst.arg3; ++i) {
                        heap[stack[inst.arg2] + i] = stack[inst.arg1 + i];
                    }
                    break;
                }
                case Op::READ_HEAP: {
                    // READ_HEAP FROM_HEAP_ADDR TO_STACK_ADDR SIZE
                    for (int i = 0; i < inst.arg3; ++i) {
                        stack[inst.arg2 + i] = heap[stack[inst.arg1] + i];
                    }
                    break;
                }
                case Op::TERM: {
                    return;
                }
            }
        }
    }

    public:
    ByteCodeVM(const Program& program) : program(program) {}

    int execute(){
        run();
        word_t ret = stacks.front().front();
        return static_cast<int>(ret);
    }

};

