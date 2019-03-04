#include <vector>

#include "gtest/gtest.h"

#include "intbig_t.h"

/*
 * Tests for comparison operators ==, !=, <, <=, >= and >
 */

// first == second
class IntBigTEqPairs : public ::testing::TestWithParam<std::pair<const intbig_t, const intbig_t>> { };

const intbig_t quite_big = intbig_t::of(1000133710000LL);

// 2 ** 64 + 1337 -- 2-limb
const intbig_t large = intbig_t::from("18446744073709552953");

// 2 ** 150 + 1414 -- 3-limb
const intbig_t huge = intbig_t::from("1427247692705959881058285969449495136382748038");

// 2 ** 633 - 1 -- 9-limb?
const intbig_t colossal = intbig_t::from("35644067325173400145634153169533525975728347712879374457649941546088087243817792082077443838416964060770643043543706307114755505635745609361348916560329798345718708393439569922522454626926591");

const std::vector<std::pair<const intbig_t, const intbig_t>> eq_pairs = {
        // 0
        { intbig_t(), intbig_t() },
        // 1-limb positive
        { intbig_t::of(1), intbig_t::of(1) },
        { intbig_t::of(100), intbig_t::of(100) },
        // 1-limb negative
        { intbig_t::of(-1), intbig_t::of(-1) },
        { intbig_t::of(-100), intbig_t::of(-100) },
        // long
        { large, large },
        { huge, huge },
        { colossal, colossal },
        { -quite_big, -quite_big },
        { quite_big + 1, quite_big + 1 }
};

TEST_P(IntBigTEqPairs, EqReturnsTrue) {
    ASSERT_EQ(GetParam().first, GetParam().second);
}

TEST_P(IntBigTEqPairs, NeReturnsFalse) {
    ASSERT_FALSE(GetParam().first != GetParam().second) << GetParam().first << " != " << GetParam().second;
}

TEST_P(IntBigTEqPairs, LtReturnsFalse) {
    ASSERT_FALSE(GetParam().first < GetParam().second) << GetParam().first << " < " << GetParam().second;
}

TEST_P(IntBigTEqPairs, LteReturnsTrue) {
    ASSERT_LE(GetParam().first, GetParam().second);
}

TEST_P(IntBigTEqPairs, GteReturnsTrue) {
    ASSERT_GE(GetParam().first, GetParam().second);
}

TEST_P(IntBigTEqPairs, GtReturnsFalse) {
    ASSERT_FALSE(GetParam().first > GetParam().second) << GetParam().first << " > " << GetParam().second;
}

INSTANTIATE_TEST_CASE_P(EqPairs, IntBigTEqPairs, ::testing::ValuesIn(eq_pairs));

// first != second
class IntBigTNePairs : public ::testing::TestWithParam<std::pair<const intbig_t, const intbig_t>> { };

const std::vector<std::pair<const intbig_t, const intbig_t>> ne_pairs = {
        // diff. value same sign
        { intbig_t::of(1), intbig_t::of(2) },
        { intbig_t(), large },
        { intbig_t(), intbig_t::of(10) },
        { intbig_t::of(-1), intbig_t() },
        { intbig_t::of(-10), intbig_t::of(11) },
        { quite_big, quite_big + 1 },
        { quite_big, colossal },
        { large, colossal },
        { large, huge },
        // same value diff. signs
        { intbig_t::of(-1), intbig_t::of(1) },
        { intbig_t::of(100), intbig_t::of(-100) },
        { intbig_t::of(-100), intbig_t::of(100) },
        { -quite_big, quite_big },
        // diff. value diff. signs
        { -quite_big - 1, -quite_big + 1 }
};

const std::vector<std::pair<const intbig_t, const intbig_t>> lt_pairs = {
        // 0
        { intbig_t::of(-1), intbig_t() },
        { -quite_big, intbig_t() },
        { intbig_t(), intbig_t::of(1) },
        { intbig_t(), quite_big },
        { intbig_t(), large },
        // diff.sign
        { intbig_t::of(-100), intbig_t::of(100) },
        { intbig_t::of(-100 + 1), intbig_t::of(100 - 1) },
        { -quite_big, intbig_t::of(100) },
        { intbig_t::of(-100), quite_big },
        { -quite_big, intbig_t::of(100) },
        { intbig_t::of(-100), colossal },
        // both positive
        { intbig_t::of(10), intbig_t::of(25) },
        { quite_big + 10, quite_big + 111 },
        { intbig_t::of(99U), intbig_t::of(100U) },
        { intbig_t::of(50), quite_big },
        { intbig_t::of(15), quite_big },
        { intbig_t::of(100), colossal },
        { large, colossal },
        { large, huge },
        // both_negative
        { intbig_t::of(-25), intbig_t::of(-10) },
        { -quite_big - 90, -quite_big - 50 },
        { intbig_t::of(-101), intbig_t::of(-10) },
        { -quite_big, intbig_t::of(-50) }
};

TEST_P(IntBigTNePairs, EqReturnsFalse) {
    ASSERT_FALSE(GetParam().first == GetParam().second) << GetParam().first << " == " << GetParam().second;
}

TEST_P(IntBigTNePairs, NeReturnsTrue) {
    ASSERT_NE(GetParam().first, GetParam().second);
    ASSERT_NE(GetParam().second, GetParam().first);
}

INSTANTIATE_TEST_CASE_P(NePairs, IntBigTNePairs, ::testing::ValuesIn(ne_pairs));
INSTANTIATE_TEST_CASE_P(LtPairs, IntBigTNePairs, ::testing::ValuesIn(lt_pairs));

// first < second
class IntBigTLtPairs : public ::testing::TestWithParam<std::pair<const intbig_t, const intbig_t>> { };

TEST_P(IntBigTLtPairs, EqReturnsFalse) {
    ASSERT_FALSE(GetParam().first == GetParam().second) << GetParam().first << " != " << GetParam().second;
}

TEST_P(IntBigTLtPairs, NeReturnsTrue) {
    ASSERT_NE(GetParam().first, GetParam().second);
}

TEST_P(IntBigTLtPairs, LtReturnsTrue) {
    ASSERT_LT(GetParam().first, GetParam().second);
}

TEST_P(IntBigTLtPairs, LteReturnsTrue) {
    ASSERT_LE(GetParam().first, GetParam().second);
}

TEST_P(IntBigTLtPairs, GteReturnsFalse) {
    ASSERT_FALSE(GetParam().first >= GetParam().second) << GetParam().first << " >= " << GetParam().second;
}

TEST_P(IntBigTLtPairs, GtReturnsFalse) {
    ASSERT_FALSE(GetParam().first > GetParam().second) << GetParam().first << " > " << GetParam().second;
}

INSTANTIATE_TEST_CASE_P(LtPairs, IntBigTLtPairs, ::testing::ValuesIn(lt_pairs));

// second > first
class IntBigTGtPairs : public ::testing::TestWithParam<std::pair<const intbig_t, const intbig_t>> { };

TEST_P(IntBigTGtPairs, EqReturnsFalse) {
    ASSERT_FALSE(GetParam().second == GetParam().first) << GetParam().second << " != " << GetParam().first;
}

TEST_P(IntBigTGtPairs, NeReturnsTrue) {
    ASSERT_NE(GetParam().second, GetParam().first);
}

TEST_P(IntBigTGtPairs, LtReturnsFalse) {
    ASSERT_FALSE(GetParam().second < GetParam().first) << GetParam().second << " < " << GetParam().first;
}

TEST_P(IntBigTGtPairs, LteReturnsFalse) {
    ASSERT_FALSE(GetParam().second <= GetParam().first) << GetParam().second << " <= " << GetParam().first;
}

TEST_P(IntBigTGtPairs, GteReturnsTrue) {
    ASSERT_GE(GetParam().second, GetParam().first);
}

TEST_P(IntBigTGtPairs, GtReturnsTrue) {
    ASSERT_GT(GetParam().second, GetParam().first);
}

INSTANTIATE_TEST_CASE_P(GtPairs, IntBigTGtPairs, ::testing::ValuesIn(lt_pairs));
