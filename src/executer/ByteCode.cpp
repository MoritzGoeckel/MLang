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
        { Op::RET, { "RET", { "NUM_PARAMS", "NUM_LOCALS" } } },
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
        { Op::LOADW, { "LOADW", { "OFFSET" } }},
        { Op::STOREW, { "STOREW", { "OFFSET" } }},
        { Op::PRINTS, {"PRINTS",{"STACK_ADDR"}} },
        { Op::TERM, {"TERM",{}} },
        { Op::LT, {"LT", {}} },
        { Op::GT, {"GT", {}} },
        { Op::EQ, {"EQ", {}} },
        { Op::LTE, {"LTE", {}} },
        { Op::GTE, {"GTE", {}} },
        { Op::NEQ, {"NEQ", {}} },
        { Op::DUB, {"DUB", {"LOOKBACK"}} },
        { Op::REG_FFI, {"REG_FFI", { "LIB_DATA_IDX", "NAME_DATA_IDX", "RETURN" }} },
        { Op::PUSH_FFI_WORD, {"PUSH_FFI_WORD", {}} },
        { Op::PUSH_FFI_DWORD, {"PUSH_FFI_DWORD", {}} },
        { Op::PUSH_FFI_QWORD, {"PUSH_FFI_QWORD", {}} },
        { Op::PUSH_FFI_XWORD, {"PUSH_FFI_XWORD", {}} },
        { Op::CALL_FFI, {"CALL_FFI", {}} },
        { Op::DATA_ADDR, {"DATA_ADDR", {"DATA_IDX"}} }
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
    for (size_t instructionCount = 0; instructionCount < maxInstructions || maxInstructions == 0; ++instructionCount) {
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
                word_t num_params = inst.arg1;
                word_t jmp_dest = stack.pop();

                // Save parameters temporarily (they're on stack in order: param0, param1, ...)
                std::vector<word_t> params;
                for (word_t i = 0; i < num_params; ++i) {
                    params.push_back(stack.pop());
                }

                stack.push(idx); // Push return address
                stack.push(function_stack_base); // Push prev function_stack_base
                function_stack_base = stack.size(); // Update function_stack_base to current top

                // Push parameters in reverse order (so param0 is at function_stack_base+0)
                for (auto it = params.rbegin(); it != params.rend(); ++it) {
                    stack.push(*it);
                }
                
                idx = jmp_dest; // Jump to function
                break;
            }
            case Op::RET: {
                word_t num_params = inst.arg1;
                word_t num_locals = inst.arg2;

                //                                           function_stack_base
                //                                                   |
                // Stack layout: [ret_addr][prev_function_stack_base]^[params...][locals...][return_value?]

                word_t expected_stack_size = function_stack_base + num_params + num_locals;
                bool has_return_value = false;
                if (stack.size() > expected_stack_size) {
                    return_value = stack.pop();
                    has_return_value = true;
                } else {
                    return_value = 0;
                }

                // Pop locals
                while (stack.size() > function_stack_base + num_params) {
                    stack.pop();
                }

                // Pop parameters
                for (word_t i = 0; i < num_params; ++i) {
                    stack.pop();
                }

                // Check if this is the main function return
                if (function_stack_base == 0) {
                    return ProgramState::Finished;
                }

                function_stack_base = stack.pop(); // Restore function_stack_base
                idx = stack.pop(); // Jump back to return address

                if (has_return_value) {
                    stack.push(return_value);
                }

                break;
            }
            case Op::LOCALS: {
                // LOCALS n: Store stack top into local variable/parameter n
                word_t value = stack.pop();

                word_t localIndex = function_stack_base + inst.arg1;

                // Expand stack if necessary
                while (stack.size() <= localIndex) {
                    stack.push(0);
                }

                stack.set(localIndex, value);
                break;
            }
            case Op::LOCALL: {
                // LOCALL n: Load local variable/parameter n onto stack
                word_t localIndex = function_stack_base + inst.arg1;

                ASSURE(localIndex < stack.size(),
                       "ByteCodeVM: Local variable index out of bounds");

                word_t value = stack.get(localIndex);
                stack.push(value);
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
                if (inst.arg1 <= 0) {
                    throwConstraintViolated("ByteCodeVM: Invalid allocation size");
                }
                // auto parentObj = stack.pop(); // TODO: Also have a parent object and garbage collection
                auto addr = heap.size();
                stack.push(addr + 1);
                auto idx = addr + inst.arg1 + 1;
                if(heap.size() <= idx) {
                    heap.resize(idx);
                }
                // heap[addr] = parentObj; // Store parent object for garbage collection
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
                auto addr = stack.pop();
                auto value = stack.pop();
                heap.at(addr + offset) = value;
                break;
            }
            case Op::DUB: {
                // DUB Num_lookback
                // Duplicate a value from the stack based on lookback index
                auto value = stack.lookback(inst.arg1);
                stack.push(value);
                break;
            }
            case Op::TERM: {
                return ProgramState::Finished;
            }
            case Op::REG_FFI: {
                // Register FFI function
                auto lib = program.data.getString(inst.arg1); // Library name
                auto name = program.data.getString(inst.arg2); // Function name
                auto retType = inst.arg3;
                auto id = ffiFunctions.add(lib, name, retType);
                stack.push(id);
                break;
            }
            case Op::PUSH_FFI_WORD:
            case Op::PUSH_FFI_DWORD:
            case Op::PUSH_FFI_QWORD: 
            case Op::PUSH_FFI_XWORD: // TODO: Distinguish between these types
            {
                auto value = stack.pop();
                ffiArgs.addQWord(value);
                break;
            }
            case Op::CALL_FFI: {
                std::cout << "Op::CALL_FFI\n";
                auto id = stack.pop();
                std::cout << "Op::CALL_FFI 1\n";
                auto result = ffiFunctions.call(id, ffiArgs);
                std::cout << "Op::CALL_FFI 2\n";
                stack.push(result);
                std::cout << "Op::CALL_FFI 3\n";
                ffiArgs.clear();
                break;
            }
            case Op::DATA_ADDR: {
                // DATA_ADDR DATA_IDX
                auto dataIdx = inst.arg1;
                void* addr = program.data.getAddr(dataIdx);
                static_assert(sizeof(word_t) == sizeof(void*));
                stack.push(reinterpret_cast<word_t>(addr));
                break;
            }
        }
    }
    return ProgramState::Paused;
}

ByteCodeVM::ByteCodeVM(const Program& program) :
    idx{0ull},
    function_stack_base{0ull},
    return_value{0ull},
    stack{},
    program(program),
    debug{true},
    ffiFunctions{},
    ffiArgs{} {}

std::string ByteCodeVM::execute(size_t maxInstructions) {
    auto state = run(maxInstructions);
    if (state != ProgramState::Finished) {
        return "Program did not finish";
    }

    if(!stack.empty()) {
        std::stringstream ss;
        while (stack.size() > 1) {
            ss << stack.pop() << ", ";
        }
        if (!stack.empty()) {
            ss << stack.back(); // Last element
        }
        return ss.str();
    }
    return "void";
}

}
