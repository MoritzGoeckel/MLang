#include "../Mlang.h"
#include "gtest/gtest.h"

TEST(Mlang, ExecuteStringSimple) {
    Mlang mlang;
    mlang.executeString("int i = 10;");
    // EXPECT_EQ(15, 1);
}

TEST(Mlang, ExecuteFile) {
    Mlang mlang;
    mlang.executeFile("mfiles/simple.m");
}

int main(int argc, char** argv) {
    Mlang::init();

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    Mlang::shutdown();
    return result;
}
