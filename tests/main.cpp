#include "gtest/gtest.h"
#include "keyboard-auto-type.h"

TEST(test_case, something) { EXPECT_EQ(2 * 2, 4); }

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
