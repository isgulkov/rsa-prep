#include <vector>
#include <random>

#include "gtest/gtest.h"
#include "InfInt.h"

#include "intbig_t.h"

/*
 * Tests for the "additive" binary operations
 * (which are pretty interconnected conceptually and implementation-wise)
 *
 * |               | In-place  | Copying   |
 * | ------------- | --------- | --------- |
 * | addition      |  +=       |  + (b)    |
 * | subtraction   |  -=       |  - (b)    |
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
 * For just the subtraction:
 *   - "large shrinkage"
 *     ones specially constructed so that the result has many digits less than the minuend
 *     the proper shrinkage (i.e. no leading zeroes) should be then checked somehow
 *     - [ ] for a - b
 *     - [ ] for b - a
 *     - [ ] zero-result cases for both
 *
 * Have I missed something? Have I included something dumb?
 *
 * NOTE: it's not tested whether each copying operator actually copies stuff as this is enforced by their return types
 *
 * Note that result comparisons are done through decimal string representation as the most "representative": most
 * human-readable and (arguably) most brittle. Tests specific to `from_decimal` and `to_decimal`, as well as other
 * representations, should be in a separate file.  (e.g. don't intentionally feed it invalid data and shit)
 *
 * TODO: make AdditiveBinaryOp a type parameter instead of value one, like in the negate tests?
 * TODO: consider employing an alternative, more respected reference -- this one doesn't inspire too much confidence
 *
 * REVIEW: make some of the elementwise asserts expects still? e.g. in the "Cases" tests
 *
 * REVIEW: type on the operation, then parametrize on the values instead of running loops?
 * REVIEW: that'd be a shitload of individual tests, though; wouldn't be able to fail early, too
 */

namespace IntBigTAdditiveBin
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

const std::vector<std::string> precise_power_positive = {
        "18446744073709551616",  // 2^64
        "6277101735386680763835789423207666416102355444464034512896"  // 2^192 = 2^(64 * 3)
};

const std::vector<std::string> precise_power_negative = {
        "-18446744073709551616",  // -2^64
        "-6277101735386680763835789423207666416102355444464034512896"  // -2^192 = -2^(64 * 3)
};

const std::vector<std::string> just_below_power_positive = {
        "18446744073709551615",  // 2^64 - 1
        "6277101735386680763835789423207666416102355444464034512895"  // 2^192 - 1
};

const std::vector<std::string> just_below_power_negative = {
        "-18446744073709551615",  // -(2^64 - 1)
        "-6277101735386680763835789423207666416102355444464034512895"  // -(2^192 - 1)
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

const std::pair<std::string, std::string> large_different_by_one = {
        "3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312415963469",
        "3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312415963468"
};
}

// Representations of the binary operators for the value-parametrized tests
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
namespace SingleTest
{

class IntBigTAdditiveSingle : public ::testing::TestWithParam<BinaryOps::AdditiveBinaryOp>
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
                        IntBigTAdditiveSingle,
                        ::testing::Values(BinaryOps::AddAssign_op)
);

INSTANTIATE_TEST_CASE_P(Add,
                        IntBigTAdditiveSingle,
                        ::testing::Values(BinaryOps::Add_op)
);

INSTANTIATE_TEST_CASE_P(SubAssign,
                        IntBigTAdditiveSingle,
                        ::testing::Values(BinaryOps::SubAssign_op)
);

INSTANTIATE_TEST_CASE_P(Sub,
                        IntBigTAdditiveSingle,
                        ::testing::Values(BinaryOps::Sub_op)
);

TEST_P(IntBigTAdditiveSingle, OperandSignCases)
{
    assertAll_labelledPairs(TestData::operand_sign_cases);
}

TEST_P(IntBigTAdditiveSingle, ZeroOperandLeft)
{
    assertAll_pairedWith(TestData::few_both, "0");
}

TEST_P(IntBigTAdditiveSingle, ZeroOperandRight)
{
    assertAll_pairedWith("0", TestData::few_both);
}

TEST_P(IntBigTAdditiveSingle, ZeroResult)
{
    assertAll_paired(TestData::few_both, TestData::few_both_opposite);
    assertAll_paired(TestData::few_both_opposite, TestData::few_both);

    expect_likeReference("0", "0");
}

TEST_P(IntBigTAdditiveSingle, PositiveSmall)
{
    assertAll_allPairs(TestData::small_positive);
}

TEST_P(IntBigTAdditiveSingle, PositiveLarge)
{
    assertAll_allPairs(TestData::large_positive);
}

TEST_P(IntBigTAdditiveSingle, NegativeSmall)
{
    assertAll_allPairs(TestData::small_negative);
}

TEST_P(IntBigTAdditiveSingle, NegativeLarge)
{
    assertAll_allPairs(TestData::large_negative);
}

TEST_P(IntBigTAdditiveSingle, MixedSignsSmall)
{
    assertAll_paired(TestData::small_negative, TestData::small_positive);
    assertAll_paired(TestData::small_positive, TestData::small_negative);
}

TEST_P(IntBigTAdditiveSingle, MixedSignsLarge)
{
    assertAll_paired(TestData::large_negative, TestData::large_positive);
    assertAll_paired(TestData::large_positive, TestData::large_negative);
}

TEST_P(IntBigTAdditiveSingle, PowerBoundaryUnderPositive)
{
    assertAll_pairedWith(TestData::just_below_power_positive, "1");
    assertAll_pairedWith(TestData::just_below_power_positive, "-1");
}

TEST_P(IntBigTAdditiveSingle, PowerBoundaryUnderNegative)
{
    assertAll_pairedWith(TestData::just_below_power_negative, "1");
    assertAll_pairedWith(TestData::just_below_power_negative, "-1");
}

TEST_P(IntBigTAdditiveSingle, PowerBoundaryOverPositive)
{
    assertAll_pairedWith(TestData::precise_power_positive, "1");
    assertAll_pairedWith(TestData::precise_power_positive, "-1");
}

TEST_P(IntBigTAdditiveSingle, PowerBoundaryOverNegative)
{
    assertAll_pairedWith(TestData::precise_power_negative, "1");
    assertAll_pairedWith(TestData::precise_power_negative, "-1");
}

}

// Tests based on applying one or more operators multiple times while checking the intermediate result
namespace ComposedTest
{

class IntBigTAdditiveComposed : public ::testing::TestWithParam<BinaryOps::AdditiveBinaryOp>
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
                        IntBigTAdditiveComposed,
                        ::testing::Values(BinaryOps::AddAssign_op)
);

INSTANTIATE_TEST_CASE_P(Add,
                        IntBigTAdditiveComposed,
                        ::testing::Values(BinaryOps::Add_op)
);

INSTANTIATE_TEST_CASE_P(SubAssign,
                        IntBigTAdditiveComposed,
                        ::testing::Values(BinaryOps::SubAssign_op)
);

INSTANTIATE_TEST_CASE_P(Sub,
                        IntBigTAdditiveComposed,
                        ::testing::Values(BinaryOps::Sub_op)
);

TEST_P(IntBigTAdditiveComposed, MonotonicPositiveOneChunk)
{
    for(std::string inc : TestData::small_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }
}

TEST_P(IntBigTAdditiveComposed, MonotonicNegativeOneChunk)
{
    for(std::string inc : TestData::small_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-" + inc));
    }
}

TEST_P(IntBigTAdditiveComposed, MonotonicPositiveManyChunks)
{
    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance(inc));
    }
}

TEST_P(IntBigTAdditiveComposed, MonotonicNegativeManyChunks)
{
    for(std::string inc : TestData::large_increments) {
        ASSERT_NO_FATAL_FAILURE(assert_advance("-" + inc));
    }
}

TEST_P(IntBigTAdditiveComposed, OscilatingManyChunks)
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

// Tests for whether subtraction shrinks the result's representation properly (i.e. removes leading zeroes)
namespace SubShrinkage
{

TEST(IntBigTAdditiveSubShrinkage, SubAssignForward)
{
    intbig_t one = intbig_t::from_decimal(TestData::large_different_by_one.first);
    one -= intbig_t::from_decimal(TestData::large_different_by_one.second);

    EXPECT_EQ(one, 1);
    EXPECT_FALSE(one > 1);
}

TEST(IntBigTAdditiveSubShrinkage, SubAssignBackward)
{
    intbig_t one = intbig_t::from_decimal(TestData::large_different_by_one.second);
    one -= intbig_t::from_decimal(TestData::large_different_by_one.first);

    EXPECT_EQ(one, -1);
    EXPECT_FALSE(one < -1);
}

TEST(IntBigTAdditiveSubShrinkage, SubAssignEqual)
{
    const std::string large = TestData::large_positive.back();

    intbig_t zero = intbig_t::from_decimal(large);
    zero -= intbig_t::from_decimal(large);

    EXPECT_EQ(zero, 0);
    EXPECT_FALSE(zero < 0);
    EXPECT_FALSE(zero > 0);
}

TEST(IntBigTAdditiveSubShrinkage, SubForward)
{
    intbig_t one = intbig_t::from_decimal(
            TestData::large_different_by_one.first
    ) - intbig_t::from_decimal(TestData::large_different_by_one.second);

    EXPECT_EQ(one, 1);
    EXPECT_FALSE(one > 1);
}

TEST(IntBigTAdditiveSubShrinkage, SubBackward)
{
    intbig_t one = intbig_t::from_decimal(
            TestData::large_different_by_one.second
    ) - intbig_t::from_decimal(TestData::large_different_by_one.first);

    EXPECT_EQ(one, -1);
    EXPECT_FALSE(one < -1);
}

TEST(IntBigTAdditiveSubShrinkage, SubEqual)
{
    const std::string large = TestData::large_positive.back();

    intbig_t zero = intbig_t::from_decimal(large) - intbig_t::from_decimal(large);

    EXPECT_EQ(zero, 0);
    EXPECT_FALSE(zero < 0);
    EXPECT_FALSE(zero > 0);
}

}

namespace AssignReturns
{

TEST(IntBigTAdditiveAssignReturns, AddAssign)
{
    intbig_t x = 10;

    ASSERT_EQ((x += 11) += 99, 120);
    ASSERT_EQ(x, 120);
}

TEST(IntBigTAdditiveAssignReturns, SubAssign)
{
    intbig_t x = 120;

    ASSERT_EQ((x -= 11) -= 99, 10);
    ASSERT_EQ(x, 10);
}

}

}
