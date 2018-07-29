#include <vector>
#include <random>

#include "gtest/gtest.h"
#include "InfInt.h"

#include "intbig_t.h"

class IntBigTComparisons : public ::testing::Test {
protected:
    void SetUp() override {
        //
    }

    void TearDown() override {
        //
    }

private:
    // BUG: this value is not actually 2-chunk anymore
    // TODO: replace (in tests) with a couple of proper long numbers when it is easy to create and copy those
    const int64_t quite_big = 1000133710000LL;

protected:
    const std::vector<std::pair<intbig_t, intbig_t>> eq_pairs = {
            // 0
            { intbig_t(), intbig_t() },
            // 1-chunk positive
            { intbig_t(1), intbig_t(1) },
            { intbig_t(100), intbig_t(100) },
            { intbig_t(99 + 1), intbig_t(100) },
            // 1-chunk negative
            { intbig_t(-1), intbig_t(-1) },
            { intbig_t(-100), intbig_t(-100) },
            { intbig_t(-99 - 1), intbig_t(-100) },
            // 2-chunk
            { intbig_t(quite_big), intbig_t(quite_big) },
            { intbig_t(-quite_big), intbig_t(-quite_big) },
            { intbig_t(quite_big + 1), intbig_t(quite_big + 1) }
    };

    const std::vector<std::pair<intbig_t, intbig_t>> ne_pairs = {
            // diff. value same sign
            { intbig_t(1), intbig_t(2) },
            { intbig_t(), intbig_t(10) },
            { intbig_t(-1), intbig_t() },
            { intbig_t(-10), intbig_t(11) },
            { intbig_t(quite_big), intbig_t(quite_big - 25U) },
            { intbig_t(-quite_big), intbig_t(-(quite_big + 10U)) },
            { intbig_t(quite_big), intbig_t(quite_big + 1) },
            // same value diff. signs
            { intbig_t(-1), intbig_t(1) },
            { intbig_t(100), intbig_t(-100) },
            { intbig_t(-100), intbig_t(100) },
            { intbig_t(-quite_big), intbig_t(quite_big) },
            { intbig_t(-quite_big - 1), intbig_t(-quite_big + 1) }
    };

    const std::vector<std::pair<intbig_t, intbig_t>> lt_pairs = {
            // 0
            { intbig_t(-1), intbig_t() },
            { intbig_t(-quite_big), intbig_t() },
            { intbig_t(), intbig_t(1) },
            { intbig_t(), intbig_t(quite_big) },
            // 1-chunk diff.sign
            { intbig_t(-100), intbig_t(100) },
            { intbig_t(-100 + 1), intbig_t(100 - 1) },
            // 1/2-chunk diff.sign
            { intbig_t(-quite_big), intbig_t(100) },
            { intbig_t(-100), intbig_t(quite_big) },
            { intbig_t(-quite_big), intbig_t(100) },
            // both positive
            { intbig_t(10), intbig_t(25) },
            { intbig_t(quite_big + 10), intbig_t(quite_big + 111) },
            { intbig_t(99U), intbig_t(100U) },
            { intbig_t(50), intbig_t(quite_big) },
            // both_negative
            { intbig_t(-25), intbig_t(-10) },
            { intbig_t(-quite_big - 90), intbig_t(-quite_big - 50) },
            { intbig_t(-101), intbig_t(-10) },
            { intbig_t(-quite_big), intbig_t(-50) },
    };

    // TODO: add 3- and 50-chunk variants when have arithmetics
};

// operator==
TEST_F(IntBigTComparisons, EqOnEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_EQ(pair.first, pair.second);
    }
}

TEST_F(IntBigTComparisons, EqOnUnequalReturnsFalse) {
    for(auto& pair : ne_pairs) {
        std::cout << pair.first << " " << pair.second << std::endl;
        EXPECT_FALSE(pair.first == pair.second);
    }

    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first == pair.second);
    }
}

// operator!=
TEST_F(IntBigTComparisons, NeOnEqualReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first != pair.second);
    }
}

TEST_F(IntBigTComparisons, NeOnUnequalReturnsTrue) {
    for(auto& pair : ne_pairs) {
        EXPECT_NE(pair.first, pair.second);
    }

    for(auto& pair : lt_pairs) {
        EXPECT_NE(pair.first, pair.second);
    }
}

// operator<
TEST_F(IntBigTComparisons, LtOnLessReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_LT(pair.first, pair.second);
    }
}

TEST_F(IntBigTComparisons, LtOnEqualReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first < pair.second);
    }
}

TEST_F(IntBigTComparisons, LtOnGreaterReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.second < pair.first);
    }
}

// operator<=
TEST_F(IntBigTComparisons, LeOnLessReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_LE(pair.first, pair.second);
    }
}

TEST_F(IntBigTComparisons, LeOnEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_LE(pair.first, pair.second);
    }
}

TEST_F(IntBigTComparisons, LeOnGreaterEqReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.second <= pair.first);
    }
}

// operator>=
TEST_F(IntBigTComparisons, GeOnLessReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first >= pair.second);
    }
}

TEST_F(IntBigTComparisons, GeOnEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_GE(pair.first, pair.second);
    }
}

TEST_F(IntBigTComparisons, GeOnGreaterEqReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_GE(pair.second, pair.first);
    }
}

// operator>
TEST_F(IntBigTComparisons, GtOnLessReturnsFalse) {
    for(auto& pair : lt_pairs) {
        EXPECT_FALSE(pair.first > pair.second);
    }
}

TEST_F(IntBigTComparisons, GtOnEqualReturnsFalse) {
    for(auto& pair : eq_pairs) {
        EXPECT_FALSE(pair.first > pair.second);
    }
}

TEST_F(IntBigTComparisons, GtOnGreaterReturnsTrue) {
    for(auto& pair : lt_pairs) {
        EXPECT_GT(pair.second, pair.first);
    }
}

// operator+=
TEST(IntBigTInPlace, AddAssignPositiveOneChunk) {
    intbig_t x(10);
    x += intbig_t(12);

    EXPECT_EQ(x, intbig_t(22));

    intbig_t y;
    y += intbig_t(233);

    EXPECT_EQ(y, intbig_t(233));

    intbig_t z(100000);
    z += intbig_t();

    EXPECT_EQ(z, intbig_t(100000));
}

TEST(IntBigTInPlace, AddAssignPositiveOneChunkMany) {
    std::mt19937 rnd(1337);

    intbig_t test;
    InfInt control;

    for(int i = 0; i < 25; i++) {
        uint32_t x = rnd() % 10000;

        if(x % 10 == 0) {
            x = 0;
        }

        control += x;
        test += intbig_t(x);

        ASSERT_EQ(control.toString(), test.to_string())
                                    << i << ": " << "+" << x << " = " << control << " " << test.to_string();
    }
}

TEST(IntBigTInPlace, AddAssignPositiveGrowth) {
    intbig_t test;
    InfInt control;

    for(int i = 0; i < 40; i++) {
        // Will go out of uint64_t range in about 8 iterations

        int64_t x = 1LL << 61;

        if(x % 10 == 0) {
            x = 0;
        }

        control += x;
        test += intbig_t(x);

        ASSERT_EQ(control.toString(), test.to_string())
                                    << i << ": " << "+" << x << " = " << control << " " << test.to_string();
    }

}
