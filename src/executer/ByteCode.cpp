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
        { Op::NOP, { "NOP", {} } },
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
        { Op::JUMP_IF, { "JUMP_IF", { "POSITIVE_ADDR" } } },
        { Op::ALLOC, { "ALLOC", { "SIZE" } } },
        { Op::WRITE_HEAP, { "WRITE_HEAP", {"FROM_STACK_ADDR", "TO_HEAP_ADDR", "SIZE"} }},
        { Op::LOADW, { "LOADW", { "OFFSET" } }},
        { Op::STOREW, { "STOREW", { "OFFSET" } }},
        { Op::READ_HEAP,  {"READ_HEAP", {"FROM_HEAP_ADDR","TO_STACK_ADDR","SIZE"} }},
        { Op::PRINTS, {"PRINTS",{"STACK_ADDR"}} },
        { Op::TERM, {"TERM",{}} },
        { Op::LT, {"LT", {}} },
        { Op::GT, {"GT", {}} },
        { Op::EQ, {"EQ", {}} },
        { Op::LTE, {"LTE", {}} },
        { Op::GTE, {"GTE", {}} },
        { Op::NEQ, {"NEQ", {}} }
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

        if(i < instructions.size() - 1) {
            ss << "\n";
        }
    }
    return ss.str();
}

ProgramState ByteCodeVM::run(size_t maxInstructions) {
    std::cout << "Running ByteCodeVM with max instructions: " << maxInstructions << std::endl;
    for (size_t instructionCount = 0; instructionCount < maxInstructions; ++instructionCount) {
        if (idx >= program.code.size()) {
            throwConstraintViolated("ByteCodeVM: Instruction index out of bounds");
        }
        const Instruction& inst = program.code[idx++];

        if(debug) {
            std::cout << "Executing instruction: " << instructionsToString({inst}, true);
            std::cout << " | Stack: ";
            for (const auto& val : stack) {
                std::cout << val << " ";
            }
            std::cout << std::endl;
        }

        switch(inst.op){
            case Op::NOP: {
                // No operation, just continue
                break;
            }
            case Op::CALL: {
                auto return_address = idx;

                auto dest = stack.pop();
                idx = dest; // Jump to function address

                // Bring parameters to locals
                std::vector<word_t> locals;
                for (int i = 0; i < inst.arg1; ++i) {
                    locals.push_back(stack.pop());
                }

                callstack.push_back({locals, return_address});
                break;
            }
            case Op::RET: {
                if(callstack.empty()){
                    return ProgramState::Finished;
                }
                idx = callstack.back().return_address;
                callstack.pop_back();
                break;
            }
            case Op::LOCALS: {
                // LOCALS ID
                auto& locals = callstack.back().locals;
                locals.resize(inst.arg1 + 1); // TODO: Expensive!

                auto value = stack.pop();
                locals[inst.arg1] = value; // Store value in local variable
                break;
            }
            case Op::LOCALL: {
                // LOCALL ID
                auto& locals = callstack.back().locals;
                stack.push(locals[inst.arg1]); // Push local variable onto stack
                break;
            }
            case Op::PRINTS: {
                std::cout << stack.pop() << std::endl;
                break;
            }
            case Op::PUSH: {
                stack.push(inst.arg1);
                break;
            }
            case Op::POP: {
                stack.pop();
                break;
            }
            case Op::WRITE_STACK: {
                throwTodo("ByteCodeVM: WRITE_STACK not implemented");
            }
            case Op::ADD: {
                // ADD RESULT_ADDR STACK_ADDR1 STACK_ADDR2
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(a + b);
                break;
            }
            case Op::SUB: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b - a);
                break;
            }
            case Op::MUL: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(a * b);
                break;
            }
            case Op::DIV: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b / a);
                break;
            }
            case Op::MOD: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b % a);
                break;
            }
            case Op::LT: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b < a ? 1 : 0);
                break;
            }
            case Op::GT: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b > a ? 1 : 0);
                break;
            }
            case Op::EQ: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b == a ? 1 : 0);
                break;
            }
            case Op::LTE: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b <= a ? 1 : 0);
                break;
            }
            case Op::GTE: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b >= a ? 1 : 0);
                break;
            }
            case Op::NEQ: {
                auto a = stack.pop();
                auto b = stack.pop();
                stack.push(b != a ? 1 : 0);
                break;
            }
            case Op::JUMP: {
                idx = inst.arg1; // Jump to address
                break;
            }
            case Op::JUMP_IF: {
                auto cond = stack.pop();
                if (cond == 0) {
                    idx = inst.arg1;
                }
                break;
            }
            case Op::ALLOC: {
                // ALLOC SIZE
                // TODO: Also have a parent object and garbage collection
                if (inst.arg1 <= 0) {
                    throwConstraintViolated("ByteCodeVM: Invalid allocation size");
                }
                stack.push(heap.size());
                heap.resize(heap.size() + inst.arg1);
                break;
            }
            case Op::WRITE_HEAP: {
                // WRITE_HEAP FROM_STACK_ADDR TO_HEAP_ADDR SIZE
                auto heap_addr = stack.pop();
                auto size = stack.pop();
                for (int i = 0; i < size; ++i) {
                    heap[heap_addr + i] = stack.pop();
                }
                break;
            }
            case Op::READ_HEAP: {
                // READ_HEAP FROM_HEAP_ADDR TO_STACK_ADDR SIZE
                auto heap_addr = stack.pop();
                auto size = stack.pop();
                for (int i = 0; i < size; ++i) {
                    stack.push(heap[heap_addr + i]);
                }
                break;
            }
            case Op::LOADW: {
                // LOADW OFFSET
                // Read from heap at stack top + offset
                auto offset = inst.arg1;
                auto addr = stack.pop();
                stack.push(heap.at(addr + offset));
                break;
            }
            case Op::STOREW: {
                // STOREW OFFSET
                // Write to heap at stack top + offset
                auto offset = inst.arg1;
                auto value = stack.pop();
                auto addr = stack.pop();
                heap.at(addr + offset) = value;
                break;
            }
            case Op::TERM: {
                return ProgramState::Finished;
            }
        }
    }
    return ProgramState::Paused;
}

ByteCodeVM::ByteCodeVM(const Program& program) : idx{0ull}, callstack{}, stack{}, program(program), debug{true} {}

std::string ByteCodeVM::execute(size_t maxInstructions) {
    auto state = run(maxInstructions);
    if (state != ProgramState::Finished) {
        return "Program did not finish";
    }

    if(stack.empty()){
        return "void";
    } else {
        std::stringstream ss;
        while (stack.size() > 1) {
            ss << stack.pop() << ", ";
        }
        if (!stack.empty()) {
            ss << stack.back(); // Last element
        }
        return ss.str();
    }
}

}
