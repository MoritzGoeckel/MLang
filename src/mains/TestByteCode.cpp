#include "../executer/ByteCode.h"
#include <iostream>

int main() {
    Program program;
    program.code = {
        Instruction(Op::PUSH_STACK, 10),
        Instruction(Op::PUSH_STACK, 20),
        Instruction(Op::ADD, 0, 1, 2),
        Instruction(Op::POP_STACK, 0),
        Instruction(Op::TERM)
    };

    // create a runner
    ByteCodeVM vm{program};
    vm.execute();

    return 0;
}