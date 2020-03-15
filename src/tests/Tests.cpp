#include "gtest/gtest.h"

TEST(Writer, WriteFileTest) { EXPECT_EQ(15, 1); }

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
