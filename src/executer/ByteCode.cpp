#include "ByteCode.h"

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <sstream>

#include "../error/Exceptions.h"

namespace executor {

Instruction::Instruction(Op op, word_t arg1, word_t arg2, word_t arg3)
        : op(op), arg1(arg1), arg2(arg2), arg3(arg3) {}

std::string instructionsToString(const std::vector<Instruction>& instructions, bool named_args) {

    static std::map<Op, OpCodeMetadata> opCodeMetadata {
        { Op::PUSH, { "PUSH", { "VALUE" } } },
        { Op::POP, { "POP", { } } },
        { Op::ADD, { "ADD", { "RESULT_ADDR", "STACK_ADDR", "STACK_ADDR" } } },
        { Op::SUB, { "SUB", { "RESULT_ADDR", "STACK_ADDR", "STACK_ADDR" } } },
        { Op::MUL, { "MUL", { "RESULT_ADDR", "STACK_ADDR", "STACK_ADDR" } } },
        { Op::DIV, { "DIV", { "RESULT_ADDR", "STACK_ADDR", "STACK_ADDR" } } },
        { Op::MOD, { "MOD", { "RESULT_ADDR", "STACK_ADDR", "STACK_ADDR" } } },
        { Op::JUMP, { "JUMP", { "ADDR" } } },
        { Op::JUMP_IF, { "JUMP_IF", { "CONDITION_STACK_ADDR", "NOT_NULL_ADDR", "NULL_ADDR" } } },
        { Op::ALLOC, { "ALLOC", { "STACK_ADDR", "SIZE" } } },
        { Op::WRITE_HEAP, { "WRITE_HEAP", {"FROM_STACK_ADDR", "TO_HEAP_ADDR", "SIZE"} }},
        { Op::READ_HEAP,  {"READ_HEAP", {"FROM_HEAP_ADDR","TO_STACK_ADDR","SIZE"} }},
        { Op::PRINTS, {"PRINTS",{"STACK_ADDR"}} },
        { Op::TERM, {"TERM",{"RET_STACK_ADDR"}} }
    };

    std::stringstream ss;
    for (const auto& inst : instructions) {
        const auto& meta = opCodeMetadata[inst.op];
        ss << meta.name << " ";
        if (named_args) {
            if (meta.arg_names.size() > 0) {
                ss << meta.arg_names[0] << "=" << inst.arg1 << " ";
            }
            if (meta.arg_names.size() > 1) {
                ss << meta.arg_names[1] << "=" << inst.arg2 << " ";
            }
            if (meta.arg_names.size() > 2) {
                ss << meta.arg_names[2] << "=" << inst.arg3 << " ";
            }
        } else {
            if(meta.arg_names.size() > 0) {
                ss << inst.arg1 << " ";
            }
            if(meta.arg_names.size() > 1) {
                ss << inst.arg2 << " ";
            }
            if(meta.arg_names.size() > 2) {
                ss << inst.arg3 << " ";
            }
        }
        ss << "\n";
    }
    return ss.str();
}


word_t ByteCodeVM::run() {
    word_t idx = 0;
    while(true){
        const Instruction& inst = program.code[idx++];
        switch(inst.op){
            case Op::PRINTS: {
                // PRINT STACK_ADDR
                std::cout << stack.back() << std::endl;
                break;
            }
            case Op::PUSH: {
                stack.emplace_back(inst.arg1);
                break;
            }
            case Op::POP: {
                stack.pop_back();
                break;
            }
            case Op::ADD: {
                // ADD RESULT_ADDR STACK_ADDR1 STACK_ADDR2
                auto a = stack.back();
                stack.pop_back();
                auto b = stack.back();
                stack.pop_back();

                stack.push_back(a + b);
                break;
            }
            case Op::SUB: {
                auto a = stack.back();
                stack.pop_back();
                auto b = stack.back();
                stack.pop_back();
                stack.push_back(b - a);
                break;
            }
            case Op::MUL: {
                auto a = stack.back();
                stack.pop_back();
                auto b = stack.back();
                stack.pop_back();
                stack.push_back(a * b);
                break;
            }
            case Op::DIV: {
                auto a = stack.back();
                stack.pop_back();
                auto b = stack.back();
                stack.pop_back();
                stack.push_back(b / a);
                break;
            }
            case Op::MOD: {
                auto a = stack.back();
                stack.pop_back();
                auto b = stack.back();
                stack.pop_back();
                stack.push_back(b % a);
                break;
            }
            case Op::JUMP: {
                idx = stack.back();
                stack.pop_back();
                break;
            }
            case Op::JUMP_IF: {
                auto cond = stack.back();
                stack.pop_back();
                auto positive = stack.back();
                stack.pop_back();
                auto negative = stack.back();
                stack.pop_back();

                if (cond != 0) {
                    idx = positive;
                } else {
                    idx = negative;
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
                if (stack.size() > 0) {
                    return stack.back();
                } else {
                    return 0;
                }
            }
        }
    }
    return 0; // Unreachable
}

ByteCodeVM::ByteCodeVM(const Program& program) : program(program) {}

int ByteCodeVM::execute(){
    run();
    word_t ret = stack.back();
    return static_cast<int>(ret);
}

}