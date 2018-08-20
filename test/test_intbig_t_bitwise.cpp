#include <vector>
#include <functional>
#include <cmath>

#include "gtest/gtest.h"

#include "intbig_t.h"

extern "C" {
#include "mini-gmp.h"
#include "mini-gmp.c"
}

/*
 * Tests for the bitwise operators
 *
 * |             | In-place | Copying |
 * | ----------- | -------- | ------- |
 * | and         |  &=      |  &      |
 * | or          |  |=      |  |      |
 * | xor         |  ^=      |  ^      |
 * | negate      |          |  ~      |
 *
 *
 *
 */

namespace TestData
{

// TODO: extract all test data into a common file

const std::vector<std::string> varied_positive = {
        "0",
        "1",
        "2",
        "10",
        "25",
        "15231",
        "10832543123",
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

std::vector<std::string> prepend_minus_(const std::vector<std::string>& xs)
{
    std::vector<std::string> negatives(xs.size());

    std::transform(xs.begin(), xs.end(), negatives.begin(), [](const std::string& x) {
        return "-" + x;
    });

    return negatives;
}

const std::vector<std::string> varied_negative = prepend_minus_(varied_positive);

}

std::string uint64_to_hex(uint64_t x)
{
    std::string result(64 / 4, '0');

    for(ssize_t i = 64 / 4 - 1; x; i--) {
        auto x_digit = uint8_t(x & 0xFU);

        x >>= 4;

        if(x_digit < 10) {
            result[i] = '0' + x_digit;
        }
        else {
            result[i] = (char)('A' - 10) + x_digit;
        }
    }

    return result;
}

std::string mpz_limbs_to_hex(const mpz_t& x)
{
    if(mpz_sgn(x) == 0) {
        return "  " + uint64_to_hex(0);
    }

    std::string s = mpz_sgn(x) == -1 ? "-" : " ";

    for(ssize_t i = mpz_size(x) - 1; i >= 0; i--) {
        s += " " + uint64_to_hex(mpz_getlimbn(x, i));
    }

    return s;
}

struct BitwiseBinaryOp
{
    const std::string op_name;
    const std::function<std::string(const std::string&, const std::string&)> get_impl;
    const std::function<std::string(const std::string&, const std::string&)> get_ref;
};

BitwiseBinaryOp BitwiseAnd_op = {
        "BitwiseAnd",
        [](const std::string& x, const std::string& y) {
            intbig_t sx = intbig_t::from_decimal(x);
            intbig_t sy = intbig_t::from_decimal(y);

            intbig_t sz = sx;
            sz &= sy;

            return sz.to_hex_chunks();
        },
        [](const std::string& x, const std::string& y) {
            mpz_t gx{}, gy{}, gz{};

            mpz_init_set_str(gx, x.c_str(), 10);
            mpz_init_set_str(gy, y.c_str(), 10);

            mpz_init(gz);
            mpz_and(gz, gx, gy);

            return mpz_limbs_to_hex(gz);
        }
};

BitwiseBinaryOp BitwiseAndCopy_op = {
        "BitwiseAndCopy",
        [](const std::string& x, const std::string& y) {
            intbig_t sx = intbig_t::from_decimal(x);
            intbig_t sy = intbig_t::from_decimal(y);

            return (sx & sy).to_hex_chunks();
        },
        [](const std::string& x, const std::string& y) {
            return BitwiseAnd_op.get_ref(x, y);
        }
};

BitwiseBinaryOp BitwiseOr_op = {
        "BitwiseOr",
        [](const std::string& x, const std::string& y) {
            intbig_t sx = intbig_t::from_decimal(x);
            intbig_t sy = intbig_t::from_decimal(y);

            intbig_t sz = sx;
            sz |= sy;

            return sz.to_hex_chunks();
        },
        [](const std::string& x, const std::string& y) {
            mpz_t gx{}, gy{}, gz{};

            mpz_init_set_str(gx, x.c_str(), 10);
            mpz_init_set_str(gy, y.c_str(), 10);

            mpz_init(gz);
            mpz_ior(gz, gx, gy);

            return mpz_limbs_to_hex(gz);
        }
};

BitwiseBinaryOp BitwiseOrCopy_op = {
        "BitwiseOrCopy",
        [](const std::string& x, const std::string& y) {
            intbig_t sx = intbig_t::from_decimal(x);
            intbig_t sy = intbig_t::from_decimal(y);

            return (sx | sy).to_hex_chunks();
        },
        [](const std::string& x, const std::string& y) {
            return BitwiseOr_op.get_ref(x, y);
        }
};

BitwiseBinaryOp BitwiseXor_op = {
        "BitwiseXor",
        [](const std::string& x, const std::string& y) {
            intbig_t sx = intbig_t::from_decimal(x);
            intbig_t sy = intbig_t::from_decimal(y);

            intbig_t sz = sx;
            sz ^= sy;

            return sz.to_hex_chunks();
        },
        [](const std::string& x, const std::string& y) {
            mpz_t gx{}, gy{}, gz{};

            mpz_init_set_str(gx, x.c_str(), 10);
            mpz_init_set_str(gy, y.c_str(), 10);

            mpz_init(gz);
            mpz_xor(gz, gx, gy);

            return mpz_limbs_to_hex(gz);
        }
};

BitwiseBinaryOp BitwiseXorCopy_op = {
        "BitwiseXorCopy",
        [](const std::string& x, const std::string& y) {
            intbig_t sx = intbig_t::from_decimal(x);
            intbig_t sy = intbig_t::from_decimal(y);

            return (sx ^ sy).to_hex_chunks();
        },
        [](const std::string& x, const std::string& y) {
            return BitwiseXor_op.get_ref(x, y);
        }
};

class IntBigTBitwiseOps : public ::testing::TestWithParam<BitwiseBinaryOp>
{
protected:
    void assert_likeReference(const std::string& x, const std::string& y)
    {
        ASSERT_EQ(
                GetParam().get_impl(x, y),
                GetParam().get_ref(x, y)
        ) << intbig_t::from_decimal(x).to_hex_chunks() << ", " << intbig_t::from_decimal(y).to_hex_chunks();
    }

    void assertAll_paired(const std::vector<std::string>& xs, const std::vector<std::string>& ys)
    {
        for(const std::string& x : xs) {
            for(const std::string& y : ys) {
                ASSERT_NO_FATAL_FAILURE(assert_likeReference(x, y));
            }
        }
    }
};

INSTANTIATE_TEST_CASE_P(BitwiseAnd, IntBigTBitwiseOps, ::testing::Values(BitwiseAnd_op));
INSTANTIATE_TEST_CASE_P(BitwiseAndCopy, IntBigTBitwiseOps, ::testing::Values(BitwiseAndCopy_op));
INSTANTIATE_TEST_CASE_P(BitwiseOr, IntBigTBitwiseOps, ::testing::Values(BitwiseOr_op));
INSTANTIATE_TEST_CASE_P(BitwiseOrCopy, IntBigTBitwiseOps, ::testing::Values(BitwiseOrCopy_op));
INSTANTIATE_TEST_CASE_P(BitwiseXor, IntBigTBitwiseOps, ::testing::Values(BitwiseXor_op));
INSTANTIATE_TEST_CASE_P(BitwiseXorCopy, IntBigTBitwiseOps, ::testing::Values(BitwiseXorCopy_op));

TEST_P(IntBigTBitwiseOps, VariedPositive)
{
    ASSERT_NO_FATAL_FAILURE(assertAll_paired(TestData::varied_positive, TestData::varied_positive));
}

TEST_P(IntBigTBitwiseOps, VariedMixedSign)
{
    ASSERT_NO_FATAL_FAILURE(assertAll_paired(TestData::varied_positive, TestData::varied_negative));
    ASSERT_NO_FATAL_FAILURE(assertAll_paired(TestData::varied_negative, TestData::varied_positive));
}

TEST_P(IntBigTBitwiseOps, VariedNegative)
{
    ASSERT_NO_FATAL_FAILURE(assertAll_paired(TestData::varied_negative, TestData::varied_negative));
}

TEST(IntBigTBitwiseInverse, UsableAsBitmask)
{
    const intbig_t x = intbig_t::of(0x0AAAAAAAAAAAAAAA);

    ASSERT_EQ(
            ((~x) & intbig_t::of(0x7FFFFFFFFFFFFFFF)).to_hex_chunks(),
            "  7555555555555555"
    );

    const intbig_t y = intbig_t::of(0x0000FF0F0000FF00);

    ASSERT_EQ(
            ((~y) & intbig_t::of(0x7FFFFFFFFFFFFFFF)).to_hex_chunks(),
            "  7FFF00F0FFFF00FF"
    );

    const intbig_t z = intbig_t::of(0x7FFFFF0FFFFFFFFF);

    ASSERT_EQ(
            ((~z & intbig_t::of(0x7FFFFFFFFFFFFFFF)) | intbig_t::of(0)).to_hex_chunks(),
            "  000000F000000000"
    );
}

TEST(IntBigTBitwiseInverse, IsOnesComplement)
{
    for(const std::string& s : TestData::varied_positive) {
        const intbig_t x = intbig_t::from_decimal(s);

        ASSERT_EQ((x + ~x).to_hex_chunks(), intbig_t::of(-1).to_hex_chunks()) << s;
        ASSERT_EQ((~~x).to_hex_chunks(), x.to_hex_chunks()) << s;
        ASSERT_EQ((x & ~x).to_hex_chunks(), intbig_t::of(0).to_hex_chunks()) << s;
        ASSERT_EQ((x | ~x).to_hex_chunks(), intbig_t::of(-1).to_hex_chunks()) << s;
        ASSERT_EQ((x ^ ~x).to_hex_chunks(), intbig_t::of(-1).to_hex_chunks()) << s;
    }
}
