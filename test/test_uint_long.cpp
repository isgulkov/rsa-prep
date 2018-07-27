#include "gtest/gtest.h"

#include "uint_long.h"

#include <vector>

// TODO: might wanna separate the test case into several and fixtures in some of them
// TODO: https://github.com/google/googletest/blob/master/googletest/docs/primer.md#test-fixtures-using-the-same-data-configuration-for-multiple-tests

class UIntLongEquality : public ::testing::Test {
protected:
    void SetUp() override {
        //
    }

    void TearDown() override {
        //
    }

private:
    const uint64_t quite_big = ((UINT32_MAX - 5U) << 10U) + 32;

protected:
    const std::vector<std::pair<uint_long, uint_long>> equal_pairs = {
            // 0
            { uint_long(), uint_long() },
            // 1-chunk positive
            { uint_long(1), uint_long(1) },
            { uint_long(100), uint_long(100) },
            { uint_long(100, false), uint_long(100) },
            // 1-chunk negative
            { uint_long(-1), uint_long(-1) },
            { uint_long(-100), uint_long(-100) },
            { uint_long(100, true), uint_long(-100) },
            { uint_long(100U, true), uint_long(100, true) },
            // 2-chunk
            { uint_long(quite_big), uint_long(quite_big) },
            { uint_long(quite_big), uint_long(quite_big, false) },
            { uint_long(quite_big, true), uint_long(-quite_big) }
    };

    const std::vector<std::pair<uint_long, uint_long>> unequal_pairs = {
            // diff. value same sign
            { uint_long(1), uint_long(2) },
            { uint_long(), uint_long(10) },
            { uint_long(-10), uint_long(11) },
            { uint_long(quite_big), uint_long(quite_big - 25U) },
            { uint_long(-quite_big), uint_long(-(quite_big + 10U)) },
            { uint_long(quite_big, true), uint_long(quite_big + 1, true) },
            // same value diff. signs
            { uint_long(-1), uint_long(1) },
            { uint_long(100), uint_long(-100) },
            { uint_long(100, true), uint_long(100) },
            { uint_long(-quite_big), uint_long(quite_big) },
            { uint_long(quite_big, true), uint_long(quite_big) },
            { uint_long(quite_big, true), uint_long(quite_big, false) }
    };
};



TEST_F(UIntLongEquality, EqEqualReturnsTrue) {
    for(auto& pair : equal_pairs) {
        EXPECT_TRUE(pair.first == pair.second);
    }
}

TEST_F(UIntLongEquality, EqUnequalReturnsFalse) {
    for(auto& pair : unequal_pairs) {
        EXPECT_FALSE(pair.first == pair.second);
    }
}

TEST_F(UIntLongEquality, NeEqualReturnsFalse) {
    for(auto& pair : equal_pairs) {
        EXPECT_FALSE(pair.first != pair.second);
    }
}

TEST_F(UIntLongEquality, NeUnequalReturnsTrue) {
    for(auto& pair : unequal_pairs) {
        EXPECT_TRUE(pair.first != pair.second);
    }
}
