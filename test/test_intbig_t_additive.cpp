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
 * | addition      |  +=      |  +       |
 * | subtraction   |  -=      |  - (bin) |
 * | negation      |  negate  |  - (un)  |
 *
 * For the binaries:
 *   - "cases":
 *     different signs of the operands and their relative magnitude;
 *   - zeroes:
 *     cases of zero operands and zero result;
 *   - positive:
 *     - small (<= 1 chunk)
 *     - mixed_sizes (2 – 100 chunks)
 *   - mixed signs:
 *     - (same)
 *   - on a chunk's brink:
 *     - 1 00(64)00 00(64)00
 *     -   11(64)11 11(64)11
 *   - "sequence":
 *     - within 1 chunk
 *     - with growth
 *     - with alternating growth and shrinkage
 *     - just some large numbers with varying sizes?
 *     - same, two ops interleaved
 *
 * For copying:
 *   - that one in fact returns a copy
 *
 * For non-copying:
 *   - that one in fact returns the same object
 *   - including when it's zero
 *
 * For negations:
 *   - some non-zero values
 *   - zero
 *
 * Have I missed something? Have I included something dumb?
 *
 * Note that result comparisons are done through decimal string representation as the most "representative". Tests
 * specific to `from_decimal` and `to_decimal`, as well as other representations, should be in a separate file.
 *   (e.g. don't feed it invalid data and shit)
 */

class IntBigTSingleBinaryOp : public ::testing::Test {
    // TODO: after += is done, find a way to abstract it all across at least the four binary ops
    // TODO: most likely parametrized, e.g. with a (name, std::function<intbig_t&>) tuple
protected:
//    virtual
    intbig_t get_result(const std::string& x, const std::string& y)
    {
        intbig_t a = intbig_t::from_decimal(x);
        a += intbig_t::from_decimal(y);

        return a;
    }

//    virtual
    std::string get_reference(const std::string& x, const std::string& y)
    {
        return (InfInt(x) + InfInt(y)).toString();
    }

    void expect_likeReference(const std::string& x, const std::string& y, const std::string& msg="")
    {
        std::string ref = get_reference(x, y);

        std::cout << (x.size() < 25 ? x : "(too long)") << " "
                  << (y.size() < 25 ? y : "(too long)") << " = "
                  << (ref.size() < 25 ? ref : "(too long)") << std::endl;

        EXPECT_EQ(
                get_result(x, y).to_string(),
                get_reference(x, y)
        ) << msg << " (" << x << " " << y << ")";
    }

    void expectAll_labelledPairs(const std::vector<std::pair<std::string, std::pair<std::string, std::string>>>& data)
    {
        for(const auto& datum : data) {
            expect_likeReference(
                    datum.second.first,
                    datum.second.second,
                    datum.first
            );
        }
    }

    void expectAll_pairedWith(const std::vector<std::string>& xs, const std::string& y)
    {
        for(const auto& x : xs) {
            expect_likeReference(x, y);
        }
    }

    void expectAll_pairedWith(const std::string& x, const std::vector<std::string>& ys)
    {
        for(const auto& y : ys) {
            expect_likeReference(x, y);
        }
    }

    void expectAll_paired(const std::vector<std::string>& xs, const std::vector<std::string>& ys)
    {
        for(auto ix = xs.begin(), iy = ys.begin(); ix != xs.end() && iy != ys.end(); ix++, iy++) {
            expect_likeReference(*ix, *iy);
        }
    }

    void expectAll_allPairs(const std::vector<std::string>& xs)
    {
        for(const auto& x : xs) {
            for(const auto& y : xs) {
                expect_likeReference(x, y);
            }
        }
    }
};

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

TEST_F(IntBigTSingleBinaryOp, OperandSignCases)
{
    expectAll_labelledPairs(IntBigTTestData::operand_sign_cases);
}

namespace IntBigTTestData
{
const std::vector<std::string> few_both = { "-48093441235234523452789014750", "-1", "10", "1999" };
const std::vector<std::string> few_both_opposite = { "48093441235234523452789014750", "1", "-10", "-1999" };
}

TEST_F(IntBigTSingleBinaryOp, ZeroOperandLeft)
{
    expectAll_pairedWith(IntBigTTestData::few_both, "0");
}

TEST_F(IntBigTSingleBinaryOp, ZeroOperandRight)
{
    expectAll_pairedWith("0", IntBigTTestData::few_both);
}

TEST_F(IntBigTSingleBinaryOp, ZeroResult)
{
    expectAll_paired(IntBigTTestData::few_both, IntBigTTestData::few_both_opposite);
    expectAll_paired(IntBigTTestData::few_both_opposite, IntBigTTestData::few_both);

    expect_likeReference("0", "0");
}

namespace IntBigTTestData {
std::vector<std::string> negated(const std::vector<std::string>& xs)
{
    // TODO: is there an STL algorithm for this?
    std::vector<std::string> negatives;

    for(const std::string& x : xs) {
        if(x != "0" && x[0] != '-') {
            negatives.push_back("-" + x);
        }
    }

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

const std::vector<std::string> small_negative = negated(small_positive);

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

const std::vector<std::string> large_negative = negated(large_positive);
}

TEST_F(IntBigTSingleBinaryOp, PositiveSmall)
{
    expectAll_allPairs(IntBigTTestData::small_positive);
}

TEST_F(IntBigTSingleBinaryOp, PositiveLarge)
{
    expectAll_allPairs(IntBigTTestData::large_positive);
}

TEST_F(IntBigTSingleBinaryOp, NegativeSmall)
{
    expectAll_allPairs(IntBigTTestData::small_negative);
}

TEST_F(IntBigTSingleBinaryOp, NegativeLarge)
{
    expectAll_allPairs(IntBigTTestData::large_negative);
}

TEST_F(IntBigTSingleBinaryOp, MixedSignsSmall)
{
    expectAll_paired(IntBigTTestData::small_negative, IntBigTTestData::small_positive);
    expectAll_paired(IntBigTTestData::small_positive, IntBigTTestData::small_negative);
}

TEST_F(IntBigTSingleBinaryOp, MixedSignsLarge)
{
    expectAll_paired(IntBigTTestData::large_negative, IntBigTTestData::large_positive);
    expectAll_paired(IntBigTTestData::large_positive, IntBigTTestData::large_negative);
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

TEST_F(IntBigTSingleBinaryOp, BrinkOfPower)
{
    expectAll_pairedWith(IntBigTTestData::precise_power, "-1");
    expectAll_pairedWith(IntBigTTestData::just_below_power, "1");
}

// REMOVE: remove after transferring their functions into the fixture
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

        int64_t x = 1ULL << 61;

        if(x % 10 == 0) {
            x = 0;
        }

        control += x;
        test += intbig_t(x);

        ASSERT_EQ(control.toString(), test.to_string())
                                    << i << ": " << "+" << x << " = " << control << " " << test.to_string();
    }
}
// REMOVE: /remove after transferring their functions into the fixture

TEST(IntBigTCopying, UnaryPlusReturnsCopy) {
    intbig_t x = 1337;
    intbig_t y = +x;

    y += 1;
    ASSERT_EQ(x, 1337);
    ASSERT_EQ(y, 1338);

    x += 2;
    ASSERT_EQ(x, 1339);
    ASSERT_EQ(y, 1338);
}

TEST(IntBigTCopying, UnaryPlusReturnsSameValue) {
    intbig_t x = 1337;

    ASSERT_EQ(x, +x);
    ASSERT_EQ(+x, 1337);

    intbig_t y = +x;

    ASSERT_EQ(y, x);
    ASSERT_EQ(y, 1337);
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

TEST(IntBigTInPlace, NegateReturnsSameObject) {
    intbig_t x = 1337;
    intbig_t& y = x.negate().negate();

    y += 1;
    ASSERT_EQ(x, 1338);
    ASSERT_EQ(y, 1338);

    x += 2;
    ASSERT_EQ(x, 1340);
    ASSERT_EQ(y, 1340);
}

TEST(IntBigTInPlace, NegateNegatesTheValue) {
    intbig_t x = 1337;

    ASSERT_EQ("-" + x.to_string(), x.negate().to_string());
    ASSERT_EQ(x, -1337);
}

TEST(IntBigTInPlace, NegatePreservesZero) {
    intbig_t x = 0;
    intbig_t y = 0;

    ASSERT_EQ(x.negate(), y);

    ASSERT_EQ(y.to_string(), y.negate().to_string());
}
