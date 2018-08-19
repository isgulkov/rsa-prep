#include <vector>

#include "gtest/gtest.h"
#include "InfInt.h"

#include "intbig_t.h"

/*
 * Tests for the unary "additive" operations
 * (which are pretty interconnected conceptually and implementation-wise)
 *
 * |               | In-place  | Copying   |
 * | ------------- | --------- | --------- |
 * | negation      |  negate   |  -        |
 * |               |           |  +        |
 * | increment     |  ++x      |  x++      |
 * | decrement     |  --x      |  x--      |
 *
 * For negations:
 *   - [x] some non-zero values
 *   - [x] zero
 *
 * For '++/--'s:
 *   - [x] sizes and signs;
 *   - [x] boundaries;
 *   - [x] compositions, including going over the sign boundary;
 *   - [x] whether the postfix ones really return a copy (i.e. the old value).
 *
 * REVIEW: type on the operation, then parametrize on the values instead of running loops?
 * REVIEW: that'd be a shitload of individual tests, though; wouldn't be able to fail early, too
 */

namespace IntBigTAdditiveUn
{

namespace TestData
{

// TODO: extract this test data into a common file

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

}

// Negation tests
namespace UnaryNegateTest
{

struct Negate_return
{
    static void apply(intbig_t& x)
    {
        x = x.negate();
    }
};

struct Negate_sameVal
{
    static void apply(intbig_t& x)
    {
        x.negate();
    }
};

struct Minus_op
{
    static void apply(intbig_t& x)
    {
        x = -x;
    }
};

template<typename NegateOp>
class IntBigTNegate : public ::testing::Test
{
public:
    void assert_negatedEq(const std::string& x, const std::string& negated_x)
    {
        intbig_t a = intbig_t::from_decimal(x);
        NegateOp::apply(a);

        ASSERT_EQ(a.to_string(), negated_x);
    }

    void assert_plusTurnsMinus(const std::string& x, const std::string& y)
    {
        intbig_t a = intbig_t::from_decimal(x);
        intbig_t b = intbig_t::from_decimal(y);

        std::string result = (a - b).to_string();

        NegateOp::apply(b);

        ASSERT_EQ((a + b).to_string(), result);
    }

    void assert_minusTurnsPlus(const std::string& x, const std::string& y)
    {
        intbig_t a = intbig_t::from_decimal(x);
        intbig_t b = intbig_t::from_decimal(y);

        std::string result = (a + b).to_string();

        NegateOp::apply(b);

        ASSERT_EQ((a - b).to_string(), result);
    }
};

using NegateOpTypes = ::testing::Types<Negate_return, Negate_sameVal, Minus_op>;
TYPED_TEST_CASE(IntBigTNegate, NegateOpTypes);

TYPED_TEST(IntBigTNegate, SmallPositive)
{
    this->assert_negatedEq("10", "-10");
    this->assert_negatedEq("999", "-999");

    this->assert_plusTurnsMinus("25", "4");
    this->assert_minusTurnsPlus("48", "15");
}

TYPED_TEST(IntBigTNegate, LargePositive)
{
    std::string large = TestData::large_positive.back();

    this->assert_negatedEq(large, "-" + large);
}

TYPED_TEST(IntBigTNegate, SmallNegative)
{
    this->assert_negatedEq("-10", "10");
    this->assert_negatedEq("-999", "999");

    this->assert_plusTurnsMinus("25", "-4");
    this->assert_minusTurnsPlus("48", "-15");
}

TYPED_TEST(IntBigTNegate, LargeNegative)
{
    std::string large = TestData::large_positive.back();

    this->assert_negatedEq("-" + large, large);
}

TYPED_TEST(IntBigTNegate, ZeroStaysZero)
{
    this->assert_negatedEq("0", "0");

    this->assert_plusTurnsMinus("25", "0");
    this->assert_minusTurnsPlus("48", "0");
}

}

// The single test for the unary plus
TEST(IntBigTUnaryPlus, UnaryPlusReturnsSameValue)
{
    intbig_t x = 1337;

    EXPECT_EQ(x, +x);

    intbig_t y = -1337;

    EXPECT_EQ(y, +y);

    intbig_t z;

    EXPECT_EQ(z, -z);
}

struct AddUnOp
{
    const std::string op_name;
    const std::function<intbig_t(const std::string&)> get_impl;
    const std::function<void(intbig_t&)> apply_impl;
    const std::function<std::string(const std::string&)> get_ref;
};

const AddUnOp IncPrefix_op = {
        "++value",
        [](const std::string& x) {
            return ++intbig_t::from_decimal(x);
        },
        [](intbig_t& x) {
            ++x;
        },
        [](const std::string& x) {
            return (++InfInt(x)).toString();
        }
};

const AddUnOp DecPrefix_op = {
        "--value",
        [](const std::string& x) {
            return --intbig_t::from_decimal(x);
        },
        [](intbig_t& x) {
            --x;
        },
        [](const std::string& x) {
            return (--InfInt(x)).toString();
        }
};

const AddUnOp IncPostfix_op = {
        "value++",
        [](const std::string& x) {
            intbig_t a = intbig_t::from_decimal(x);
            a++;
            return a;
        },
        [](intbig_t& x) {
            x++;
        },
        [](const std::string& x) {
            InfInt a = x;
            a++;
            return a.toString();
        }
};

const AddUnOp DecPostfix_op = {
        "value--",
        [](const std::string& x) {
            intbig_t a = intbig_t::from_decimal(x);
            a--;
            return a;
        },
        [](intbig_t& x) {
            x--;
        },
        [](const std::string& x) {
            InfInt a = x;
            a--;
            return a.toString();
        }
};

class IntBigTAddUnSingle : public ::testing::TestWithParam<AddUnOp>
{
protected:
    void assert_likeReference(const std::string& x, const std::string& msg = "")
    {
        ASSERT_EQ(
                GetParam().get_impl(x).to_string(),
                GetParam().get_ref(x)
        ) << msg << " (" << x << ")";
    }

    void assertAll_likeReference(const std::vector<std::string>& xs)
    {
        for(const std::string& x : xs) {
            ASSERT_NO_FATAL_FAILURE(
                    assert_likeReference(x)
            );
        }
    }
};

INSTANTIATE_TEST_CASE_P(
        IncPrefix,
        IntBigTAddUnSingle,
        ::testing::Values(IncPrefix_op)
);

INSTANTIATE_TEST_CASE_P(
        DecPrefix,
        IntBigTAddUnSingle,
        ::testing::Values(DecPrefix_op)
);

INSTANTIATE_TEST_CASE_P(
        IncPostfix,
        IntBigTAddUnSingle,
        ::testing::Values(IncPostfix_op)
);

INSTANTIATE_TEST_CASE_P(
        DecPostfix,
        IntBigTAddUnSingle,
        ::testing::Values(DecPostfix_op)
);

TEST_P(IntBigTAddUnSingle, SmallPositive)
{
    assertAll_likeReference(TestData::small_positive);
}

TEST_P(IntBigTAddUnSingle, ZeroArgument)
{
    assert_likeReference("0");
}

TEST_P(IntBigTAddUnSingle, SmallNegative)
{
    assertAll_likeReference(TestData::small_negative);
}

TEST_P(IntBigTAddUnSingle, LargePositive)
{
    assertAll_likeReference(TestData::large_positive);
}

TEST_P(IntBigTAddUnSingle, LargeNegative)
{
    assertAll_likeReference(TestData::large_negative);
}

TEST_P(IntBigTAddUnSingle, PowerBoundaryUnderPositive)
{
    assertAll_likeReference(TestData::just_below_power_positive);
}

TEST_P(IntBigTAddUnSingle, PowerBoundaryUnderNegative)
{
    assertAll_likeReference(TestData::just_below_power_negative);
}

TEST_P(IntBigTAddUnSingle, PowerBoundaryOverPositive)
{
    assertAll_likeReference(TestData::precise_power_positive);
}

TEST_P(IntBigTAddUnSingle, PowerBoundaryOverNegative)
{
    assertAll_likeReference(TestData::precise_power_negative);
}

TEST(IntBigTAddUnPostfix, IncPostfixReturnsOldValue)
{
    intbig_t x = intbig_t::from_decimal("10");

    ASSERT_EQ((x++).to_string(), "10");
    ASSERT_EQ(x.to_string(), "11");
}

TEST(IntBigTAddUnPostfix, DecPostfixReturnsOldValue)
{
    intbig_t x = intbig_t::from_decimal("10");

    ASSERT_EQ((x--).to_string(), "10");
    ASSERT_EQ(x.to_string(), "9");
}

class IntBigTAddUnComposed : public ::testing::TestWithParam<std::vector<int>>
{
protected:
    intbig_t x;

    void SetUp()
    {
        x = intbig_t();
    }

    void assert_likeReference(const AddUnOp& op)
    {
        std::string ref = op.get_ref(x.to_string());

        op.apply_impl(x);

        ASSERT_EQ(x.to_string(), ref);
    }

    void assertMultiple_likeReference(const AddUnOp& op, int n_times)
    {
        for(int i = 0; i < n_times; i++) {
            ASSERT_NO_FATAL_FAILURE(assert_likeReference(op));
        }
    }
};

TEST_F(IntBigTAddUnComposed, MultipleZeroCrossings)
{
    ASSERT_NO_FATAL_FAILURE(assertMultiple_likeReference(IncPrefix_op, 10));

    ASSERT_NO_FATAL_FAILURE(assertMultiple_likeReference(DecPostfix_op, 15));

    ASSERT_NO_FATAL_FAILURE(assertMultiple_likeReference(IncPostfix_op, 20));

    ASSERT_NO_FATAL_FAILURE(assertMultiple_likeReference(DecPrefix_op, 25));

    ASSERT_NO_FATAL_FAILURE(assertMultiple_likeReference(IncPrefix_op, 30));
}

}
