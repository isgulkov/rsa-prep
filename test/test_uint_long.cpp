#include <vector>
#include <random>

#include "gtest/gtest.h"
#include "gmpxx.h"

#include "uint_long.h"

// TODO: might wanna separate the test case into several and fixtures in some of them
// TODO: https://github.com/google/googletest/blob/master/googletest/docs/primer.md#test-fixtures-using-the-same-data-configuration-for-multiple-tests

class UIntLongComparisons : public ::testing::Test {
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
    const std::vector<std::pair<uint_long, uint_long>> eq_pairs = {
            // 0
            { uint_long(), uint_long() },
            // 1-chunk positive
            { uint_long(1), uint_long(1) },
            { uint_long(100), uint_long(100) },
            { uint_long(100U, false), uint_long(100) },
            // 1-chunk negative
            { uint_long(-1), uint_long(-1) },
            { uint_long(-100), uint_long(-100) },
            { uint_long(100U, true), uint_long(-100) },
            { uint_long(100U, true), uint_long(100U, true) },
            // 2-chunk
            { uint_long(quite_big), uint_long(quite_big) },
            { uint_long(quite_big), uint_long(quite_big, false) },
            { uint_long(quite_big, true), uint_long(-quite_big) }
    };

    const std::vector<std::pair<uint_long, uint_long>> ne_pairs = {
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
            { uint_long(100U, true), uint_long(100) },
            { uint_long(-quite_big), uint_long(quite_big) },
            { uint_long(quite_big, true), uint_long(quite_big) },
            { uint_long(quite_big, true), uint_long(quite_big, false) }
    };

    const std::vector<std::pair<uint_long, uint_long>> lt_pairs = {
            // 0
            { uint_long(-1), uint_long() },
            { uint_long(-quite_big), uint_long() },
            { uint_long(), uint_long(1) },
            { uint_long(), uint_long(quite_big) },
            // 1-chunk diff.sign
            { uint_long(-100), uint_long(100) },
            { uint_long(100U, true), uint_long(100) },
            { uint_long(-100), uint_long(100U, false) },
            // 1/2-chunk diff.sign
            { uint_long(-quite_big), uint_long(100) },
            { uint_long(100U, true), uint_long(quite_big) },
            { uint_long(-quite_big), uint_long(100U, false) },
            // both positive
            { uint_long(10), uint_long(25) },
            { uint_long(quite_big + 10), uint_long(quite_big + 111) },
            { uint_long(99U, false), uint_long(100U , false) },
            { uint_long(50), uint_long(quite_big, false) },
            // both_negative
            { uint_long(-25), uint_long(-10) },
            { uint_long(-quite_big - 90), uint_long(-quite_big - 50) },
            { uint_long(101U, true), uint_long(10U , true) },
            { uint_long(quite_big, true), uint_long(50) },
    };

    // TODO: add 3- and 50-chunk variants when have arithmetics
};

// operator==
TEST_F(UIntLongComparisons, EqEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_TRUE(pair.first == pair.second);
    }
}

TEST_F(UIntLongComparisons, EqUnequalReturnsFalse) {
    for(auto& pair : ne_pairs) {
        EXPECT_FALSE(pair.first == pair.second);
    }
}

// operator!=
TEST_F(UIntLongComparisons, NeEqualReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first != pair.second);
    }
}

TEST_F(UIntLongComparisons, NeUnequalReturnsTrue) {
    for(auto& pair : ne_pairs) {
        EXPECT_TRUE(pair.first != pair.second);
    }
}

// operator<
TEST_F(UIntLongComparisons, LtLessReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.first < pair.second);
    }
}

TEST_F(UIntLongComparisons, EqLessReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first < pair.second);
    }
}

TEST_F(UIntLongComparisons, GtLessReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.second < pair.first);
    }
}

// operator<=
TEST_F(UIntLongComparisons, LtLessEqReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.first <= pair.second);
    }
}

TEST_F(UIntLongComparisons, EqLessEqReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_TRUE(pair.first <= pair.second);
    }
}

TEST_F(UIntLongComparisons, GtLessEqReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.second <= pair.first);
    }
}

// operator>=
TEST_F(UIntLongComparisons, LtGreaterEqReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first >= pair.second);
    }
}

TEST_F(UIntLongComparisons, EqGreaterEqReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_TRUE(pair.first >= pair.second);
    }
}

TEST_F(UIntLongComparisons, GtGreaterEqReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.second >= pair.first);
    }
}

// operator>
TEST_F(UIntLongComparisons, LtGreaterReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first > pair.second);
    }
}

TEST_F(UIntLongComparisons, EqGreaterReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first > pair.second);
    }
}

TEST_F(UIntLongComparisons, GtGreaterReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.second > pair.first);
    }
}

// operator+=
TEST(UIntLongInPlace, PlusEqPosOneChunk) {
    uint_long x(10);
    x += uint_long(12);

    EXPECT_TRUE(x == uint_long(22));

    uint_long y;
    y += uint_long(233);

    EXPECT_TRUE(y == uint_long(233));

    uint_long z(100000, false);
    z += uint_long();

    EXPECT_TRUE(z == uint_long(100000));
}

TEST(UIntLongInPlace, PlusEqPosOneChunkMany) {
    std::mt19937 rnd(1337);

    uint_long test;
    mpz_class control;

    for(int i = 0; i < 25; i++) {
        uint32_t x = rnd() % 10000;

        if(x % 10 == 0) {
            x = 0;
        }

        control += x;
        test += uint_long(x);

        ASSERT_EQ(cmp(control, (int32_t)test), 0) << "+" << x << " = " << control << " " << (int32_t)test;
    }
}
