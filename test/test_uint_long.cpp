#include "gtest/gtest.h"

#include "uint_long.h"

// TODO: might wanna separate the test case into several and fixtures in some of them
// TODO: https://github.com/google/googletest/blob/master/googletest/docs/primer.md#test-fixtures-using-the-same-data-configuration-for-multiple-tests

TEST(LongInt, HelloWorldGreen) {
    ASSERT_EQ(hello(), "Hello, world!");
    ASSERT_EQ(hello() + " Hui.", "Hello, world! Hui.");
}

TEST(LongInt, HelloWorldRed) {
    ASSERT_EQ(hello(), "Hello, world.");
    ASSERT_EQ(hello(), "Hello, world.") << "You don't see this";
}

TEST(LongInt, HelloWorldDoubleRed) {
    EXPECT_EQ(hello(), "Hello, world?") << "I'm not your world, universe!";
    EXPECT_EQ(hello(), "Hello, universe!") << "I'm not your universe, cosmos!";
}
