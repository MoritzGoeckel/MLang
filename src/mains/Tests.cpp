#include "../executer/ByteCode.h"
#include <iostream>

void TEST_ByteCodeVM() {
    Program program;
    program.code = {
        Instruction(Op::PUSH_STACK, 2),
        Instruction(Op::WRITE_STACK, 0 /*dest_addr*/, 12 /*value*/),
        Instruction(Op::WRITE_STACK, 1 /*dest_addr*/, 3 /*value*/),
        Instruction(Op::MUL, 0 /*dest_addr*/, 0 /*src_addr*/, 1 /*src_addr*/),
        Instruction(Op::POP_STACK, 0 /*dest_addr*/, 0 /*src_addr*/),
        Instruction(Op::TERM, 0 /*src_addr*/)
    };

    ByteCodeVM vm{program};
    int result = vm.execute();
    std::cout << "Result: " << result << std::endl;
}

int main() {
    TEST_ByteCodeVM();
    return 0;
}
