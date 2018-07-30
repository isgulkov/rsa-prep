#include <vector>
#include <random>

#include "gtest/gtest.h"
#include "InfInt.h"

#include "intbig_t.h"

/*
 * Tests for additive operations, including both the in-place and the copying versions:
 *   - addition:
 *       - operator+=
 *       - operator+
 *   - subtract:
 *       - operator-=
 *       - operator- (binary)
 *   - negation:
 *       - negate()
 *       - operator- (unary)
 *
 * (these are pretty interconnected internally)
 */

class IntBigTBinaryAdditive : public ::testing::Test {
protected:
    struct single_triple
    {
        std::string a, b, sum;
    };

    std::vector<single_triple> triples;

    void SetUp() override {
        /*
         * This ideally should be redone and divided into a number of categories for separate test cases.
         * TODO: categorize the inputs (maybe write a test generator in Python?..)
         *
         * Categories:
         *   - size (n chunks): 1, several (2–10), lots (50–250)
         *   - sign: +, -, zero
         *   - some special small values: 1, 2, 10, 100
         *   - ... ??? ...
         *
         * The main question, though, is: how do I mix them? Don't want to end up with *too* many tests
         */

        // TODO: set this up once (somehow)

        const std::vector<std::string> argumentos = {
                "0",
                "1",
                "2",
                "10",
                "999",
                "1000133710000",
                "18446744073709552953",
                "1427247692705959881058285969449495136382748038",
                "356440673251734001456341531695335259757283477128793744576499415460880872438177920820774438384169640"
                "60770643043543706307114755505635745609361348916560329798345718708393439569922522454626926591"
        };

        triples.clear();

        for(const auto& x : argumentos) {
            for(const auto& y : argumentos) {
                triples.push_back({ x, y, (InfInt(x) + InfInt(y)).toString() });
            }
        }
    }

    void TearDown() override {
        //
    }
};

void assert_AddAssignResult(intbig_t a, intbig_t b, std::string s, std::string msg)
{
    a += b;
    ASSERT_EQ(a.to_string(), s) << msg;
}

void expect_AddAssignResult(intbig_t a, intbig_t b, std::string s, std::string msg)
{
    a += b;
    EXPECT_EQ(a.to_string(), s) << msg;
}

TEST(IntBigTInPlace, AddAssignCases) {
    std::vector<std::pair<std::string, std::pair<int64_t, int64_t>>> cases = {
            { "P+P", { 14, 18 } },
            { "P+N<", { 10, -15 } },
            { "P+N>", { 15, -10 } },
            { "N+P<", { -10, 15 } },
            { "N+P>", { -15, 10 } },
            { "N+N", { 33, 11 } }
    };

    for(const auto& priv : cases) {
        intbig_t x = priv.second.first;
        x += priv.second.second;

        assert_AddAssignResult(
                priv.second.first,
                priv.second.second,
                std::to_string(priv.second.first + priv.second.second),
                priv.first
        );
    }
}

TEST(IntBigTInPlace, AddAssignZeroes) {
    expect_AddAssignResult({}, -1, "-1", "a-");
    expect_AddAssignResult({}, 11, "11", "a+");
    expect_AddAssignResult(-40, {}, "-40", "-b");
    expect_AddAssignResult(97, {}, "97", "+b");
    expect_AddAssignResult(-5, 5, "0", "result-+");
    expect_AddAssignResult(87, -87, "0", "result+-");
    expect_AddAssignResult({}, {}, "0", "HAT TRICK!");
}

TEST_F(IntBigTBinaryAdditive, AddAssignTriples) {
    for(const single_triple& triple : triples) {
        expect_AddAssignResult(
                intbig_t::from_decimal(triple.a),
                intbig_t::from_decimal(triple.b),
                triple.sum,
                triple.a + " " + triple.b
        );
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

// operator-=
TEST(IntBigTInPlace, SubAssignPositiveOneChunk) {
    intbig_t x(22);
    x -= intbig_t(10);

    EXPECT_EQ(x, intbig_t(12));

    intbig_t y(233);

    y -= 0;
    EXPECT_EQ(y, intbig_t(233));

    y -= 233;
    EXPECT_EQ(y, intbig_t(0));

    intbig_t z;
    z -= intbig_t(-1111);

    EXPECT_EQ(z, intbig_t(1111));
}

// TODO: somehow generalize inplace tests onto copying tests
// operator+
//TEST(IntBigTCopying, AddPositiveOneChunk) {
//    intbig_t x(10);
//
//    EXPECT_EQ(x + 12, intbig_t(22));
//
//    intbig_t y;
//
//    EXPECT_EQ(y + 233, intbig_t(233));
//
//    intbig_t z(100000);
//
//    EXPECT_EQ(z + 0, intbig_t(100000));
//
//    // TODO: the other way around (e.g. 10 + x)
//}

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