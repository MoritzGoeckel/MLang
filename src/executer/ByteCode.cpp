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
        { Op::LOCALS, { "LOCALS", { "ID" } } },
        { Op::LOCALL, { "LOCALL", { "ID" } } },
        { Op::CALL, { "CALL", { "NUM_ARGS" } } },
        { Op::RET, { "RET", { } } },
        { Op::PUSH, { "PUSH", { "VALUE" } } },
        { Op::POP, { "POP", {} } },
        { Op::ADD, { "ADD", {} } },
        { Op::SUB, { "SUB", {} } },
        { Op::MUL, { "MUL", {} } },
        { Op::DIV, { "DIV", {} } },
        { Op::MOD, { "MOD", {} } },
        { Op::JUMP, { "JUMP", { "ADDR" } } },
        { Op::JUMP_IF, { "JUMP_IF", {} } },
        { Op::ALLOC, { "ALLOC", { "STACK_ADDR", "SIZE" } } },
        { Op::WRITE_HEAP, { "WRITE_HEAP", {"FROM_STACK_ADDR", "TO_HEAP_ADDR", "SIZE"} }},
        { Op::READ_HEAP,  {"READ_HEAP", {"FROM_HEAP_ADDR","TO_STACK_ADDR","SIZE"} }},
        { Op::PRINTS, {"PRINTS",{"STACK_ADDR"}} },
        { Op::TERM, {"TERM",{}} }
    };

    std::stringstream ss;
    for (int i = 0; i < instructions.size(); ++i) {
        const Instruction& inst = instructions[i];
        ss << i << ": ";
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

struct StackFrame {
    std::vector<word_t> locals;
    word_t return_address; 
};

bool ByteCodeVM::run() {
    word_t idx = 0;
    std::vector<StackFrame> callstack;

    while(true){
        const Instruction& inst = program.code[idx++];
        switch(inst.op){
            case Op::CALL: {
                auto return_address = idx;

                auto dest = stack.back();
                stack.pop_back();
                idx = dest; // Jump to function address

                // Bring parameters to locals
                std::vector<word_t> locals;
                for (int i = 0; i < inst.arg1; ++i) {
                    locals.push_back(stack.back());
                    stack.pop_back();           
                }

                callstack.push_back({locals, return_address});
                break;
            }
            case Op::RET: {
                if(callstack.empty()){
                    return true;
                }
                idx = callstack.back().return_address;
                callstack.pop_back();
                break;
            }
            case Op::LOCALS: {
                // LOCALS ID
                auto& locals = callstack.back().locals;
                locals.resize(inst.arg1); // TODO: Expensive!

                auto value = stack.back();
                stack.pop_back();

                locals[inst.arg1] = value; // Store value in local variable
                break;
            }
            case Op::LOCALL: {
                // LOCALL ID
                auto& locals = callstack.back().locals;
                stack.push_back(locals[inst.arg1]); // Push local variable onto stack
                break;
            }
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
                idx = inst.arg1; // Jump to address
                break;
            }
            case Op::JUMP_IF: {
                auto cond = stack.back();
                stack.pop_back();

                if (cond != 0) {
                    idx = inst.arg1;
                } else {
                    idx = inst.arg1;
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
                return true;
            }
        }
    }
    return 0; // Unreachable
}

ByteCodeVM::ByteCodeVM(const Program& program) : program(program) {}

int ByteCodeVM::execute(){
    run();
    if(stack.empty()){
        return 0;
    } else {
        return static_cast<int>(stack.back());
    }
}

}