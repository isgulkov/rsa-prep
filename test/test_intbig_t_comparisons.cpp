#include <vector>

#include "gtest/gtest.h"

#include "intbig_t.h"

/*
 * Tests for comparison `operator`s ==, !=, <, <=, >= and >
 */

class IntBigTComparisons : public ::testing::Test {
protected:
    void SetUp() override {
        //
    }

    void TearDown() override {
        //
    }

private:
    // TODO: After the unary minus and non-compound +/- are done,
    // TODO: replace each use of this number with a couple of analogous ones of some of the three numbers below
    const int64_t quite_big = 1000133710000LL;

    // TODO: after << is done, create these numbers algebraically rather than from strings
    // 2 ** 64 + 1337 -- 2-limb
    const intbig_t large = intbig_t::from_decimal("18446744073709552953");

    // 2 ** 150 + 1414 -- 3-limb
    const intbig_t huge = intbig_t::from_decimal("1427247692705959881058285969449495136382748038");

    // 2 ** 633 - 1 -- 9-limb?
    const intbig_t colossal = intbig_t::from_decimal("35644067325173400145634153169533525975728347712879374457649941546088087243817792082077443838416964060770643043543706307114755505635745609361348916560329798345718708393439569922522454626926591");

    // TODO: replace everything below (0, 1, 99, 100, etc) with constants declared over here

protected:
    const std::vector<std::pair<intbig_t, intbig_t>> eq_pairs = {
            // 0
            { intbig_t(), intbig_t() },
            // 1-limb positive
            { intbig_t(1), intbig_t(1) },
            { intbig_t(100), intbig_t(100) },
            { intbig_t(99 + 1), intbig_t(100) },
            // 1-limb negative
            { intbig_t(-1), intbig_t(-1) },
            { intbig_t(-100), intbig_t(-100) },
            { intbig_t(-99 - 1), intbig_t(-100) },
            // long
            { large, large },
            { huge, huge },
            { colossal, colossal },
            { intbig_t(-quite_big), intbig_t(-quite_big) },
            { intbig_t(quite_big + 1), intbig_t(quite_big + 1) }
    };

    const std::vector<std::pair<intbig_t, intbig_t>> ne_pairs = {
            // diff. value same sign
            { intbig_t(1), intbig_t(2) },
            { intbig_t(), large },
            { intbig_t(), intbig_t(10) },
            { colossal, intbig_t(10) },
            { intbig_t(-1), intbig_t() },
            { intbig_t(-10), intbig_t(11) },
            { intbig_t(quite_big), intbig_t(quite_big - 25U) },
            { intbig_t(-quite_big), intbig_t(-(quite_big + 10U)) },
            { intbig_t(quite_big), intbig_t(quite_big + 1) },
            { intbig_t(quite_big), colossal },
            { large, colossal },
            { large, huge },
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
            { intbig_t(), large },
            // diff.sign
            { intbig_t(-100), intbig_t(100) },
            { intbig_t(-100 + 1), intbig_t(100 - 1) },
            { intbig_t(-quite_big), intbig_t(100) },
            { intbig_t(-100), intbig_t(quite_big) },
            { intbig_t(-quite_big), intbig_t(100) },
            { intbig_t(-100), colossal },
            // both positive
            { intbig_t(10), intbig_t(25) },
            { intbig_t(quite_big + 10), intbig_t(quite_big + 111) },
            { intbig_t(99U), intbig_t(100U) },
            { intbig_t(50), intbig_t(quite_big) },
            { intbig_t(15), intbig_t(quite_big) },
            { intbig_t(100), colossal },
            { large, colossal },
            { large, huge },
            // both_negative
            { intbig_t(-25), intbig_t(-10) },
            { intbig_t(-quite_big - 90), intbig_t(-quite_big - 50) },
            { intbig_t(-101), intbig_t(-10) },
            { intbig_t(-quite_big), intbig_t(-50) },
    };
};

// operator==
TEST_F(IntBigTComparisons, EqOnEqualReturnsTrue) {
    for(auto& pair : eq_pairs) {
        EXPECT_EQ(pair.first, pair.second);
    }
}

TEST_F(IntBigTComparisons, EqOnUnequalReturnsFalse) {
    for(auto& pair : ne_pairs) {
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
