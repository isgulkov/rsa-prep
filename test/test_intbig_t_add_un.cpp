#include <vector>
#include <random>

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
 * | inc./dec.     |  ++/--    | ++(int)   |
 * |               |           | / --(int) |
 *
 * For negations:
 *   - [x] some non-zero values
 *   - [x] zero
 *
 * TODO: consider employing an alternative, more respected reference -- this one doesn't inspire too much confidence
 *
 * REVIEW: type on the operation, then parametrize on the values instead of running loops?
 * REVIEW: that'd be a shitload of individual tests, though; wouldn't be able to fail early, too
 */

namespace IntBigTAdditiveUn
{

// Test data for the whole file go here
namespace TestData
{

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

}
