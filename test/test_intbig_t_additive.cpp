#include <vector>
#include <random>

#include "gtest/gtest.h"
#include "InfInt.h"

#include "intbig_t.h"

/*
 * Tests for the "additive" operations
 * (which are pretty interconnected conceptually and implementation-wise)
 *
 * |               | In-place | Copying  |
 * | ------------- | -------- | ---------|
 * | addition      |  +=      |  + (b)   |
 * | subtraction   |  -=      |  - (b)   |
 * | negation      |  negate  |  - (u)   |
 * |               |          |  + (u)   |
 *
 * For the binaries:
 *   - [x] "cases":
 *         different signs of the operands and their relative magnitude;
 *   - [x] zeroes:
 *     cases of zero operands and zero result;
 *   - [x] positive:
 *         - [x] small (<= 1 chunk)
 *         - [x] large (2 – 100 chunks)
 *   - [x] mixed signs:
 *         - [x] (same)
 *   - [x] on a chunk's edge:
 *         - 1 00(64)00 00(64)00
 *         -   11(64)11 11(64)11
 *   - [x] composition:
 *         - [x] within 1 chunk
 *         - [x] with growth
 *         - [x] with alternating growth and shrinkage
 *
 * For negations:
 *   - [ ] some non-zero values
 *   - [ ] zero
 *
 * Have I missed something? Have I included something dumb?
 *
 * NOTE: it's not tested whether each copying operator actually copies stuff as this is enforced by their return types
 *
 * Note that result comparisons are done through decimal string representation as the most "representative": most
 * human-readable and (arguably) most brittle. Tests specific to `from_decimal` and `to_decimal`, as well as other
 * representations, should be in a separate file.  (e.g. don't intentionally feed it invalid data and shit)
 *
 * TODO: put the "Single" and "Composed" at the end of the fixture names
 * TODO: put some notion of "Additive" in the test names
 *
 * TODO: this whole file should be reorganized -- test data, function structs and all
 * TODO: unify the single and composable structs? like, their get_reference methods are basically identical
 *
 * TODO: propagate ASSERT failures with ASSERT_NO_FATAL_FAILURE everywhere appropriate
 * TODO: (the output of multiple errors makes some failing tests run very slow)
 *
 * REVIEW: type on the operation, then parametrize on the values instead of running loops?
 * REVIEW: that'd be a shitload of tests, though
 */

namespace
{
struct BinaryOpSingleTester  // TODO: better name for this and the other "...Tester"
{
    std::string op_name;
    std::function<intbig_t(const std::string&, const std::string&)> get_result;
    std::function<std::string(const std::string&, const std::string&)> get_reference;
};
}

class IntBigTAdditiveBinarySingle : public ::testing::TestWithParam<BinaryOpSingleTester>
{
protected:
    intbig_t get_result(const std::string& x, const std::string& y)
    {
        return GetParam().get_result(x, y);
    }

    std::string get_reference(const std::string& x, const std::string& y)
    {
        return GetParam().get_reference(x, y);
    }

    void expect_likeReference(const std::string& x, const std::string& y, const std::string& msg="")
    {
//        std::string ref = get_reference(x, y);
//        std::cout << (x.size() < 25 ? x : "(too long)") << " "
//                  << (y.size() < 25 ? y : "(too long)") << " = "
//                  << (ref.size() < 25 ? ref : "(too long)") << std::endl;

        EXPECT_EQ(
                get_result(x, y).to_string(),
                get_reference(x, y)
        ) << msg << " (" << x << " " << y << ")";
    }

    void assert_likeReference(const std::string& x, const std::string& y, const std::string& msg="")
    {
        ASSERT_EQ(
                get_result(x, y).to_string(),
                get_reference(x, y)
        ) << msg << " (" << x << " " << y << ")";
    }

    void assertAll_labelledPairs(const std::vector<std::pair<std::string, std::pair<std::string, std::string>>>& data)
    {
        for(const auto& datum : data) {
            ASSERT_NO_FATAL_FAILURE(
                    assert_likeReference(
                        datum.second.first,
                        datum.second.second,
                        datum.first
                )
            );
        }
    }

    void assertAll_pairedWith(const std::vector<std::string>& xs, const std::string& y)
    {
        for(const auto& x : xs) {
            ASSERT_NO_FATAL_FAILURE(assert_likeReference(x, y));
        }
    }

    void assertAll_pairedWith(const std::string& x, const std::vector<std::string>& ys)
    {
        for(const auto& y : ys) {
            ASSERT_NO_FATAL_FAILURE(assert_likeReference(x, y));
        }
    }

    void assertAll_paired(const std::vector<std::string>& xs, const std::vector<std::string>& ys)
    {
        for(auto ix = xs.begin(), iy = ys.begin(); ix != xs.end() && iy != ys.end(); ix++, iy++) {
            ASSERT_NO_FATAL_FAILURE(assert_likeReference(*ix, *iy));
        }
    }

    void assertAll_allPairs(const std::vector<std::string>& xs)
    {
        for(const auto& x : xs) {
            for(const auto& y : xs) {
                ASSERT_NO_FATAL_FAILURE(assert_likeReference(x, y));
            }
        }
    }
};

namespace {
const BinaryOpSingleTester AddAssign_single = {
        "AddAssign",
        [](const std::string& x, const std::string& y) {
            intbig_t a = intbig_t::from_decimal(x);
            a += intbig_t::from_decimal(y);

            return a;
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) + InfInt(y)).toString();
        }
};

const BinaryOpSingleTester Add_single = {
        "Add",
        [](const std::string& x, const std::string& y) {
            return intbig_t::from_decimal(x) + intbig_t::from_decimal(y);
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) + InfInt(y)).toString();
        }
};

const BinaryOpSingleTester SubAssign_single = {
        "SubAssign",
        [](const std::string& x, const std::string& y) {
            intbig_t a = intbig_t::from_decimal(x);
            a -= intbig_t::from_decimal(y);

            return a;
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) - InfInt(y)).toString();
        }
};

const BinaryOpSingleTester Sub_single = {
        "Sub",
        [](const std::string& x, const std::string& y) {
            return intbig_t::from_decimal(x) - intbig_t::from_decimal(y);
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) - InfInt(y)).toString();
        }
};
}

INSTANTIATE_TEST_CASE_P(AddAssign,
                        IntBigTAdditiveBinarySingle,
                        ::testing::Values(AddAssign_single)
);

INSTANTIATE_TEST_CASE_P(Add,
                        IntBigTAdditiveBinarySingle,
                        ::testing::Values(Add_single)
);

INSTANTIATE_TEST_CASE_P(SubAssign,
                        IntBigTAdditiveBinarySingle,
                        ::testing::Values(SubAssign_single)
);

INSTANTIATE_TEST_CASE_P(Sub,
                        IntBigTAdditiveBinarySingle,
                        ::testing::Values(Sub_single)
);

namespace IntBigTTestData
{
const std::vector<std::pair<std::string, std::pair<std::string, std::string>>> operand_sign_cases = {
        { "P+P<", { "14",   "18" }},
        { "P+P>", { "140",  "5" }},
        { "P+N<", { "10",   "-15" }},
        { "P+N>", { "15",   "-10" }},
        { "N+P<", { "-10",  "15" }},
        { "N+P>", { "-15",  "10" }},
        { "N+N<", { "-111", "-333" }},
        { "N+N>", { "-33",  "-11" }}
};
}

TEST_P(IntBigTAdditiveBinarySingle, OperandSignCases)
{
    assertAll_labelledPairs(IntBigTTestData::operand_sign_cases);
}

namespace IntBigTTestData
{
const std::vector<std::string> few_both = { "-48093441235234523452789014750", "-1", "10", "1999" };
const std::vector<std::string> few_both_opposite = { "48093441235234523452789014750", "1", "-10", "-1999" };
}

TEST_P(IntBigTAdditiveBinarySingle, ZeroOperandLeft)
{
    assertAll_pairedWith(IntBigTTestData::few_both, "0");
}

TEST_P(IntBigTAdditiveBinarySingle, ZeroOperandRight)
{
    assertAll_pairedWith("0", IntBigTTestData::few_both);
}

TEST_P(IntBigTAdditiveBinarySingle, ZeroResult)
{
    assertAll_paired(IntBigTTestData::few_both, IntBigTTestData::few_both_opposite);
    assertAll_paired(IntBigTTestData::few_both_opposite, IntBigTTestData::few_both);

    expect_likeReference("0", "0");
}

namespace IntBigTTestData {
std::vector<std::string> prepend_minus(const std::vector<std::string>& xs)
{
    std::vector<std::string> negatives(xs.size());

    std::transform(xs.begin(), xs.end(), negatives.begin(), [](const std::string& x) {
        return "-" + x;
    });

    return negatives;
}

const std::vector<std::string> small_positive = {
        "1",
        "2",
        "10",
        "25",
        "15231",
        "10832543123"
};

const std::vector<std::string> small_negative = prepend_minus(small_positive);

// NOTE: 2^64 is about 10^19
const std::vector<std::string> large_positive = {
        "87972214896678166229" // NOLINT(misc-suspicious-missing-comma)
        "8173139348",
        "78697751883288514847"
        "84972777418658240764"
        "20388401460",
        "57423592486388743291"
        "20382180194200746563"
        "00425999022736303907"
        "0",
        "31618394871677635941"
        "07724496196279505125"
        "21311374778669464226"
        "18080836213935927656"
        "16154012046788175944"
        "49793356331241596346"
        "2",
        "91535470444892758731"
        "19721219676782834081"
        "70366383734444150489"
        "98737636821178635335"
        "19824378316354308866"
        "81029633517773110583"
        "03072121395638867659"
        "15641689607841664929"
        "19824378316354308866"
        "35848181009425536864"
        "67651698652459"
};

const std::vector<std::string> large_negative = prepend_minus(large_positive);
}

TEST_P(IntBigTAdditiveBinarySingle, PositiveSmall)
{
    assertAll_allPairs(IntBigTTestData::small_positive);
}

TEST_P(IntBigTAdditiveBinarySingle, PositiveLarge)
{
    assertAll_allPairs(IntBigTTestData::large_positive);
}

TEST_P(IntBigTAdditiveBinarySingle, NegativeSmall)
{
    assertAll_allPairs(IntBigTTestData::small_negative);
}

TEST_P(IntBigTAdditiveBinarySingle, NegativeLarge)
{
    assertAll_allPairs(IntBigTTestData::large_negative);
}

TEST_P(IntBigTAdditiveBinarySingle, MixedSignsSmall)
{
    assertAll_paired(IntBigTTestData::small_negative, IntBigTTestData::small_positive);
    assertAll_paired(IntBigTTestData::small_positive, IntBigTTestData::small_negative);
}

TEST_P(IntBigTAdditiveBinarySingle, MixedSignsLarge)
{
    assertAll_paired(IntBigTTestData::large_negative, IntBigTTestData::large_positive);
    assertAll_paired(IntBigTTestData::large_positive, IntBigTTestData::large_negative);
}

namespace IntBigTTestData {
const std::vector<std::string> precise_power = {
         "18446744073709551616",  // 2^64
        "-18446744073709551616",
         "6277101735386680763835789423207666416102355444464034512896",  // 2^192 = 2^(64 * 3)
        "-6277101735386680763835789423207666416102355444464034512896"
};

const std::vector<std::string> just_below_power = {
         "18446744073709551615",  // 2^64 - 1
        "-18446744073709551615",
         "6277101735386680763835789423207666416102355444464034512895",  // 2^192 - 1
        "-6277101735386680763835789423207666416102355444464034512895"
};
}

TEST_P(IntBigTAdditiveBinarySingle, PowerBoundaryUnderPosOne)
{
    assertAll_pairedWith(IntBigTTestData::just_below_power, "1");
}

TEST_P(IntBigTAdditiveBinarySingle, PowerBoundaryUnderNegOne)
{
    assertAll_pairedWith(IntBigTTestData::just_below_power, "-1");
}

TEST_P(IntBigTAdditiveBinarySingle, PowerBoundaryOverPosOne)
{
    assertAll_pairedWith(IntBigTTestData::precise_power, "1");
}

TEST_P(IntBigTAdditiveBinarySingle, PowerBoundaryOverNegOne)
{
    assertAll_pairedWith(IntBigTTestData::precise_power, "-1");
}

namespace {
struct BinaryOpComposableTester {
    std::string op_name;
    std::function<void(intbig_t&, const std::string&)> apply_impl;
    std::function<std::string(intbig_t&, const std::string&)> apply_ref;
};

const BinaryOpComposableTester AddAssign_composable = {
        "AddAssign",
        [](intbig_t& x, const std::string& arg) {
            x += intbig_t::from_decimal(arg);
        },
        [](intbig_t& x, const std::string& arg) {
            return (InfInt(x.to_string()) + InfInt(arg)).toString();
        }
};

const BinaryOpComposableTester Add_composable = {
        "Add",
        [](intbig_t& x, const std::string& arg) {
            x = x + intbig_t::from_decimal(arg);
        },
        [](intbig_t& x, const std::string& arg) {
            return (InfInt(x.to_string()) + InfInt(arg)).toString();
        }
};

const BinaryOpComposableTester SubAssign_composable = {
        "SubAssign",
        [](intbig_t& x, const std::string& arg) {
            x -= intbig_t::from_decimal(arg);
        },
        [](intbig_t& x, const std::string& arg) {
            return (InfInt(x.to_string()) - InfInt(arg)).toString();
        }
};

const BinaryOpComposableTester Sub_composable = {
        "Sub",
        [](intbig_t& x, const std::string& arg) {
            x = x - intbig_t::from_decimal(arg);
        },
        [](intbig_t& x, const std::string& arg) {
            return (InfInt(x.to_string()) - InfInt(arg)).toString();
        }
};

const std::vector<BinaryOpComposableTester> composable_ops = {
        AddAssign_composable,
        Add_composable,
        SubAssign_composable,
        Sub_composable
};
}

class IntBigTAdditiveBinaryComposed : public ::testing::TestWithParam<BinaryOpComposableTester>
{
protected:
    intbig_t current;
    size_t i_iter;

    void SetUp() override {
        current = {};
        i_iter = 0;
    }

    void assert_advance(const std::string& x)
    {
        std::string ref = GetParam().apply_ref(current, x);

        GetParam().apply_impl(current, x);

        ASSERT_EQ(
                current.to_string(), ref
        ) << " after arg. " << x << " on iteration " << i_iter;

        i_iter += 1;
    }
};

INSTANTIATE_TEST_CASE_P(AddAssign,
                        IntBigTAdditiveBinaryComposed,
                        ::testing::Values(AddAssign_composable)
);

INSTANTIATE_TEST_CASE_P(Add,
                        IntBigTAdditiveBinaryComposed,
                        ::testing::Values(Add_composable)
);

INSTANTIATE_TEST_CASE_P(SubAssign,
                        IntBigTAdditiveBinaryComposed,
                        ::testing::Values(SubAssign_composable)
);

INSTANTIATE_TEST_CASE_P(Sub,
                        IntBigTAdditiveBinaryComposed,
                        ::testing::Values(Sub_composable)
);

namespace {
// since C++20: std::generate
std::vector<std::string> gen_increments(int64_t x, int64_t b)
{
    std::vector<std::string> increments(20);

    InfInt increment = 11;

    for(auto it = increments.begin(); it != increments.end(); it++) {
        *it = increment.toString();

        increment *= x;
        increment += b;
    }

    return increments;
}

const std::vector<std::string> small_increments = gen_increments(11, -19);

const std::vector<std::string> large_increments = gen_increments(17171, -18);
}

TEST_P(IntBigTAdditiveBinaryComposed, MonotonicPositiveOneChunk)
{
    // TODO: check against alternative reference -- this may very well be the Vasyan's fault

    for(std::string inc : small_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }
}

TEST_P(IntBigTAdditiveBinaryComposed, MonotonicNegativeOneChunk)
{
    for(std::string inc : small_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-" + inc));
    }
}

TEST_P(IntBigTAdditiveBinaryComposed, MonotonicPositiveManyChunks)
{
    for(std::string inc : large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }
}

TEST_P(IntBigTAdditiveBinaryComposed, MonotonicNegativeManyChunks)
{
    for(std::string inc : large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-" + inc));
    }
}

TEST_P(IntBigTAdditiveBinaryComposed, OscilatingManyChunks)
{
    for(std::string inc : large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }

    for(std::string inc : large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-1" + inc));
    }

    for(std::string inc : large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("2" + inc));
    }

    for(std::string inc : large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-4" + inc));
    }
}

// TODO: rework or amend the unary tests below

TEST(IntBigTCopying, UnaryPlusReturnsCopy) {
    intbig_t x = 1337;
    intbig_t y = +x;

    y += 1;

    EXPECT_NE(x, y);
}

TEST(IntBigTCopying, UnaryPlusReturnsSameValue) {
    intbig_t x = 1337;

    EXPECT_EQ(x, +x);

    intbig_t y;

    EXPECT_EQ(x, +y);
}

TEST(IntBigTCopying, UnaryMinusReturnsCopy) {
    // TODO: needs +=/-= for negatives

    intbig_t x = 1337;
    intbig_t y = -x;

    y += 1;
    ASSERT_EQ(x, 1337);
    ASSERT_EQ(y, -1336);

    x += 2;
    ASSERT_EQ(x, 1339);
    ASSERT_EQ(y, -1336);
}

TEST(IntBigTCopying, UnaryMinusReturnsOppositeValue) {
    intbig_t x = 1337;

    ASSERT_EQ((-x).to_string(), "-" + x.to_string());
    ASSERT_EQ(-x, -1337);

    intbig_t y = -x;

    ASSERT_EQ(y.to_string(), "-" + x.to_string());
    ASSERT_EQ(y, -1337);
}

TEST(IntBigTCopying, UnaryMinusOfZeroReturnsSameValue) {
    intbig_t x = 0;

    ASSERT_EQ((-x), x);
    ASSERT_EQ(-x, 0);
}

TEST(IntBigTInPlace, NegateNegatesTheValue) {
    intbig_t x = 1337;

    EXPECT_EQ("-" + x.to_string(), x.negate().to_string());
    EXPECT_EQ(x, -1337);
}

TEST(IntBigTInPlace, NegatePreservesZero) {
    intbig_t x = 0;
    intbig_t y = 0;

    ASSERT_EQ(x.negate(), y);

    ASSERT_EQ(y.to_string(), y.negate().to_string());
}
