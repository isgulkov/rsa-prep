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
 * TODO: make AdditiveBinaryOp a type parameter instead of value one
 *
 * TODO: consider employing an alternative, more respected reference -- this one doesn't inspire too much confidence
 *
 * REVIEW: make some of the elementwise asserts expects still? e.g. in the "Cases" tests
 *
 * REVIEW: type on the operation, then parametrize on the values instead of running loops?
 * REVIEW: that'd be a shitload of individual tests, though; wouldn't be able to fail early, too
 */

namespace IntBigTAdditive
{

// Test data for the whole file go here
namespace TestData
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

const std::vector<std::string> few_both = { "-48093441235234523452789014750", "-1", "10", "1999" };
const std::vector<std::string> few_both_opposite = { "48093441235234523452789014750", "1", "-10", "-1999" };

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

// Representations of the binary operators for the data-parametrized tests
namespace BinaryOps
{
struct AdditiveBinaryOp
{
    const std::string op_name;
    const std::function<intbig_t(const std::string&, const std::string&)> get_impl;
    const std::function<void(intbig_t&, const std::string&)> apply_impl;
    const std::function<std::string(const std::string&, const std::string&)> get_ref;
};

const AdditiveBinaryOp AddAssign_op = {
        "AddAssign",
        [](const std::string& x, const std::string& y) {
            intbig_t a = intbig_t::from_decimal(x);
            a += intbig_t::from_decimal(y);

            return a;
        },
        [](intbig_t& x, const std::string& y) {
            x += intbig_t::from_decimal(y);
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) + InfInt(y)).toString();
        }
};

const AdditiveBinaryOp Add_op = {
        "Add",
        [](const std::string& x, const std::string& y) {
            return intbig_t::from_decimal(x) + intbig_t::from_decimal(y);
        },
        [](intbig_t& x, const std::string& y) {
            x = x + intbig_t::from_decimal(y);
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) + InfInt(y)).toString();
        }
};

const AdditiveBinaryOp SubAssign_op = {
        "SubAssign",
        [](const std::string& x, const std::string& y) {
            intbig_t a = intbig_t::from_decimal(x);
            a -= intbig_t::from_decimal(y);

            return a;
        },
        [](intbig_t& x, const std::string& y) {
            x -= intbig_t::from_decimal(y);
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) - InfInt(y)).toString();
        }
};

const AdditiveBinaryOp Sub_op = {
        "Sub",
        [](const std::string& x, const std::string& y) {
            return intbig_t::from_decimal(x) - intbig_t::from_decimal(y);
        },
        [](intbig_t& x, const std::string& y) {
            x = x - intbig_t::from_decimal(y);
        },
        [](const std::string& x, const std::string& y) {
            return (InfInt(x) - InfInt(y)).toString();
        }
};
}

// Tests based on single, isolated application of an operator
namespace BinarySingleTest
{

class SingleOpTestCase : public ::testing::TestWithParam<BinaryOps::AdditiveBinaryOp>
{
protected:
    intbig_t get_result(const std::string& x, const std::string& y)
    {
        return GetParam().get_impl(x, y);
    }

    std::string get_reference(const std::string& x, const std::string& y)
    {
        return GetParam().get_ref(x, y);
    }

    void expect_likeReference(const std::string& x, const std::string& y, const std::string& msg = "")
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

    void assert_likeReference(const std::string& x, const std::string& y, const std::string& msg = "")
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

INSTANTIATE_TEST_CASE_P(AddAssign,
                        SingleOpTestCase,
                        ::testing::Values(BinaryOps::AddAssign_op)
);

INSTANTIATE_TEST_CASE_P(Add,
                        SingleOpTestCase,
                        ::testing::Values(BinaryOps::Add_op)
);

INSTANTIATE_TEST_CASE_P(SubAssign,
                        SingleOpTestCase,
                        ::testing::Values(BinaryOps::SubAssign_op)
);

INSTANTIATE_TEST_CASE_P(Sub,
                        SingleOpTestCase,
                        ::testing::Values(BinaryOps::Sub_op)
);

TEST_P(SingleOpTestCase, OperandSignCases)
{
    assertAll_labelledPairs(TestData::operand_sign_cases);
}

TEST_P(SingleOpTestCase, ZeroOperandLeft)
{
    assertAll_pairedWith(TestData::few_both, "0");
}

TEST_P(SingleOpTestCase, ZeroOperandRight)
{
    assertAll_pairedWith("0", TestData::few_both);
}

TEST_P(SingleOpTestCase, ZeroResult)
{
    assertAll_paired(TestData::few_both, TestData::few_both_opposite);
    assertAll_paired(TestData::few_both_opposite, TestData::few_both);

    expect_likeReference("0", "0");
}

TEST_P(SingleOpTestCase, PositiveSmall)
{
    assertAll_allPairs(TestData::small_positive);
}

TEST_P(SingleOpTestCase, PositiveLarge)
{
    assertAll_allPairs(TestData::large_positive);
}

TEST_P(SingleOpTestCase, NegativeSmall)
{
    assertAll_allPairs(TestData::small_negative);
}

TEST_P(SingleOpTestCase, NegativeLarge)
{
    assertAll_allPairs(TestData::large_negative);
}

TEST_P(SingleOpTestCase, MixedSignsSmall)
{
    assertAll_paired(TestData::small_negative, TestData::small_positive);
    assertAll_paired(TestData::small_positive, TestData::small_negative);
}

TEST_P(SingleOpTestCase, MixedSignsLarge)
{
    assertAll_paired(TestData::large_negative, TestData::large_positive);
    assertAll_paired(TestData::large_positive, TestData::large_negative);
}

TEST_P(SingleOpTestCase, PowerBoundaryUnderPosOne)
{
    assertAll_pairedWith(TestData::just_below_power, "1");
}

TEST_P(SingleOpTestCase, PowerBoundaryUnderNegOne)
{
    assertAll_pairedWith(TestData::just_below_power, "-1");
}

TEST_P(SingleOpTestCase, PowerBoundaryOverPosOne)
{
    assertAll_pairedWith(TestData::precise_power, "1");
}

TEST_P(SingleOpTestCase, PowerBoundaryOverNegOne)
{
    assertAll_pairedWith(TestData::precise_power, "-1");
}

}

// Tests based on applying one or more operators multiple times while checking the intermediate result
namespace BinaryComposedTest
{

class ComposedOpTestCase : public ::testing::TestWithParam<BinaryOps::AdditiveBinaryOp>
{
protected:
    intbig_t current;
    size_t i_iter;

    void SetUp() override
    {
        current = { };
        i_iter = 0;
    }

    void assert_advance(const std::string& x)
    {
        std::string ref = GetParam().get_ref(current.to_string(), x);

        GetParam().apply_impl(current, x);

        ASSERT_EQ(
                current.to_string(), ref
        ) << " after arg. " << x << " on iteration " << i_iter;

        i_iter += 1;
    }
};

INSTANTIATE_TEST_CASE_P(AddAssign,
                        ComposedOpTestCase,
                        ::testing::Values(BinaryOps::AddAssign_op)
);

INSTANTIATE_TEST_CASE_P(Add,
                        ComposedOpTestCase,
                        ::testing::Values(BinaryOps::Add_op)
);

INSTANTIATE_TEST_CASE_P(SubAssign,
                        ComposedOpTestCase,
                        ::testing::Values(BinaryOps::SubAssign_op)
);

INSTANTIATE_TEST_CASE_P(Sub,
                        ComposedOpTestCase,
                        ::testing::Values(BinaryOps::Sub_op)
);

TEST_P(ComposedOpTestCase, MonotonicPositiveOneChunk)
{
    for(std::string inc : TestData::small_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }
}

TEST_P(ComposedOpTestCase, MonotonicNegativeOneChunk)
{
    for(std::string inc : TestData::small_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-" + inc));
    }
}

TEST_P(ComposedOpTestCase, MonotonicPositiveManyChunks)
{
    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }
}

TEST_P(ComposedOpTestCase, MonotonicNegativeManyChunks)
{
    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-" + inc));
    }
}

TEST_P(ComposedOpTestCase, OscilatingManyChunks)
{
    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }

    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-1" + inc));
    }

    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("2" + inc));
    }

    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-4" + inc));
    }
}

}

// TODO: rework or amend the unary tests below

TEST(IntBigTCopying, UnaryPlusReturnsCopy)
{
    intbig_t x = 1337;
    intbig_t y = +x;

    y += 1;

    EXPECT_NE(x, y);
}

TEST(IntBigTCopying, UnaryPlusReturnsSameValue)
{
    intbig_t x = 1337;

    EXPECT_EQ(x, +x);

    intbig_t y;

    EXPECT_EQ(x, +y);
}

TEST(IntBigTCopying, UnaryMinusReturnsCopy)
{
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

TEST(IntBigTCopying, UnaryMinusReturnsOppositeValue)
{
    intbig_t x = 1337;

    ASSERT_EQ((-x).to_string(), "-" + x.to_string());
    ASSERT_EQ(-x, -1337);

    intbig_t y = -x;

    ASSERT_EQ(y.to_string(), "-" + x.to_string());
    ASSERT_EQ(y, -1337);
}

TEST(IntBigTCopying, UnaryMinusOfZeroReturnsSameValue)
{
    intbig_t x = 0;

    ASSERT_EQ((-x), x);
    ASSERT_EQ(-x, 0);
}

TEST(IntBigTInPlace, NegateNegatesTheValue)
{
    intbig_t x = 1337;

    EXPECT_EQ("-" + x.to_string(), x.negate().to_string());
    EXPECT_EQ(x, -1337);
}

TEST(IntBigTInPlace, NegatePreservesZero)
{
    intbig_t x = 0;
    intbig_t y = 0;

    ASSERT_EQ(x.negate(), y);

    ASSERT_EQ(y.to_string(), y.negate().to_string());
}

}
