#include "../executer/ByteCode.h"
#include <iostream>

#define EXPECT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::cerr << __FILE__ << ":" << __LINE__  << " " << __FUNCTION__ << " Expected: " << (expected) << ", but got: " << (actual) << std::endl; \
        throw std::runtime_error("Test failed!"); \
    }

void TEST_ByteCodeVM() {
    using namespace executor;

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
    EXPECT_EQ(36, result); // 12 * 3 = 36
}

void TEST_Addition(){

}

int main() {
    TEST_ByteCodeVM();
    return 0;
}
