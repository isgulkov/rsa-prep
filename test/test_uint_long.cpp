#include <vector>
#include <random>

#include "gtest/gtest.h"
#include "InfInt.h"

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
TEST_F(UIntLongComparisons, EqOnEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_TRUE(pair.first == pair.second);
    }
}

TEST_F(UIntLongComparisons, EqOnUnequalReturnsFalse) {
    for(auto& pair : ne_pairs) {
        EXPECT_FALSE(pair.first == pair.second);
    }

    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first == pair.second);
    }
}

// operator!=
TEST_F(UIntLongComparisons, NeOnEqualReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first != pair.second);
    }
}

TEST_F(UIntLongComparisons, NeOnUnequalReturnsTrue) {
    for(auto& pair : ne_pairs) {
        EXPECT_TRUE(pair.first != pair.second);
    }

    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first == pair.second);
    }
}

// operator<
TEST_F(UIntLongComparisons, LtOnLessReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.first < pair.second);
    }
}

TEST_F(UIntLongComparisons, LtOnEqualReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first < pair.second);
    }
}

TEST_F(UIntLongComparisons, LtOnGreaterReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.second < pair.first);
    }
}

// operator<=
TEST_F(UIntLongComparisons, LeOnLessReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.first <= pair.second);
    }
}

TEST_F(UIntLongComparisons, LeOnEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_TRUE(pair.first <= pair.second);
    }
}

TEST_F(UIntLongComparisons, LeOnGreaterEqReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.second <= pair.first);
    }
}

// operator>=
TEST_F(UIntLongComparisons, GeOnLessReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first >= pair.second);
    }
}

TEST_F(UIntLongComparisons, GeOnEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_TRUE(pair.first >= pair.second);
    }
}

TEST_F(UIntLongComparisons, GeOnGreaterEqReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.second >= pair.first);
    }
}

// operator>
TEST_F(UIntLongComparisons, GtOnLessReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first > pair.second);
    }
}

TEST_F(UIntLongComparisons, GtOnEqualReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first > pair.second);
    }
}

TEST_F(UIntLongComparisons, GtOnGreaterReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_TRUE(pair.second > pair.first);
    }
}

// operator+=
TEST(UIntLongInPlace, AddAssignPositiveOneChunk) {
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

TEST(UIntLongInPlace, AddAssignPositiveOneChunkMany) {
    std::mt19937 rnd(1337);

    uint_long test;
    InfInt control;

    for(int i = 0; i < 25; i++) {
        uint32_t x = rnd() % 10000;

        if(x % 10 == 0) {
            x = 0;
        }

        control += x;
        test += uint_long(x);

        ASSERT_EQ(control.toString(), test.to_string())
                                    << i << ": " << "+" << x << " = " << control << " " << test.to_string();
    }
}

TEST(UIntLongInPlace, AddAssignPositiveGrowth) {
    std::mt19937 rnd(1337);
    std::exponential_distribution<double> exp(1.0 / 10000000);

    uint_long test;
    InfInt control;

    for(int i = 0; i < 1000; i++) {
        auto x = (uint32_t)exp(rnd);

        if(x % 10 == 0) {
            x = 0;
        }

        control += x;
        test += uint_long(x);

        ASSERT_EQ(control.toString(), test.to_string())
                                    << i << ": " << "+" << x << " = " << control << " " << test.to_string();
    }

}
