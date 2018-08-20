#include <vector>

#include "gtest/gtest.h"
#include "InfInt.h"

#include "intbig_t.h"

/*
 * Tests for the bit shift operators
 *
 * |             | In-place | Copying |
 * | ----------- | -------- | ------- |
 * | left shift  |  <<=     |  <<     |
 * | right shift |  >>=     |  >>     |
 *
 * For shifts:
 *   - [x] various shifts of large and small numbers;
 *   - [x] shifts resulting in zero;
 *   - [x] shifts of zero;
 *   - [x] shift by negative results in symmetric shift by absolute;
 *   - [x] shifts of negatives behave like corresponding shifts of their absolutes;
 *           - [x] except that right shift is idempotent for -1 instead of 0;
 *   - [x] something basic for the copying shifts.
 *
 * TODO: remove "bitwise" from the names in this file
 *
 */

namespace TestData
{

// TODO: extract this test data into a common file

const std::vector<std::string> small_positive = {
        "1",
        "2",
        "10",
        "25",
        "15231",
        "10832543123"
};

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

const std::vector<int> positive_shifts = { 0, 1, 8, 37, 63, 64, 65, 130, 1000 };

std::vector<std::string> prepend_minus(const std::vector<std::string>& xs)
{
    std::vector<std::string> negatives(xs.size());

    std::transform(xs.begin(), xs.end(), negatives.begin(), [](const std::string& x) {
        return "-" + x;
    });

    return negatives;
}

const std::vector<std::string> small_negative = prepend_minus(small_positive);
const std::vector<std::string> large_negative = prepend_minus(large_positive);

}

enum ShiftDir { LEFT, RIGHT };

class IntBigTBitwiseShifts : public ::testing::TestWithParam<ShiftDir>
{
protected:
    void assert_likeReference(const std::string& s, int n)
    {
        intbig_t x = intbig_t::from_decimal(s);
        InfInt y = s;

        if(GetParam() == ShiftDir::LEFT) {
            x <<= n;

            for(int i = 0; i < n; i++) {
                y *= 2;
            }
        }
        else if(GetParam() == ShiftDir::RIGHT) {
            x >>= n;

            for(int i = 0; i < n; i++) {
                if(y >= 0) {
                    y /= 2;
                }
                else {
                    y /= 2;

                    if(y == 0) {
                        y = "-1";
                    }
                }
            }
        }
        else {
            FAIL() << "Unknown direction: " << GetParam();
        }

        ASSERT_EQ(x.to_string(), y.toString()) << intbig_t::from_decimal(s).to_hex_chunks()
                                               << (GetParam() == ShiftDir::LEFT ? " << " : " >> ") << n;
    }

    void assertAll_paired(const std::vector<std::string>& xs, const std::vector<int>& ns)
    {
        for(const std::string& x : xs) {
            for(const int n : ns) {
                ASSERT_NO_FATAL_FAILURE(assert_likeReference(x, n));
            }
        }
    }
};

INSTANTIATE_TEST_CASE_P(LeftShift, IntBigTBitwiseShifts, ::testing::Values(ShiftDir::LEFT));

INSTANTIATE_TEST_CASE_P(RightShift, IntBigTBitwiseShifts, ::testing::Values(ShiftDir::RIGHT));

TEST_P(IntBigTBitwiseShifts, PosSmallByPosVarious)
{
    ASSERT_NO_FATAL_FAILURE(
            assertAll_paired(
                    TestData::small_positive, TestData::positive_shifts
            )
    );
}

TEST_P(IntBigTBitwiseShifts, PosLargeByPosVarious)
{
    ASSERT_NO_FATAL_FAILURE(
            assertAll_paired(
                    TestData::large_positive, TestData::positive_shifts
            )
    );
}

TEST_P(IntBigTBitwiseShifts, PosByItsNumBits)
{
    for(const std::string& x : TestData::large_positive) {
        int n_bits = (int)intbig_t::from_decimal(x).num_bits();

        ASSERT_NO_FATAL_FAILURE(
                assert_likeReference(x, n_bits)
        );

        ASSERT_NO_FATAL_FAILURE(
                assert_likeReference(x, n_bits - 1)
        );
    }
}

TEST_P(IntBigTBitwiseShifts, ZeroByPosVarious)
{
    ASSERT_NO_FATAL_FAILURE(
            assertAll_paired(
                    { "0" },
                    { 0, 1, 8, 80 }
            )
    );
}

TEST(IntBigTBitwiseShifts, PosByNegativeResultsInSymmetric)
{
    for(const int n : TestData::positive_shifts) {
        intbig_t x;

        if(n % 2) {
            x = intbig_t::from_decimal(TestData::small_positive[n % TestData::small_positive.size()]);
        }
        else {
            x = intbig_t::from_decimal(TestData::large_positive[n % TestData::large_positive.size()]);
        }

        intbig_t y = x;
        intbig_t z = x;

        y <<= -n;
        z >>= n;

        ASSERT_EQ(y.to_string(), z.to_string()) << x.to_string() << " by " << n;

        y = x;
        z = x;

        y >>= -n;
        z <<= n;

        ASSERT_EQ(y.to_string(), z.to_string()) << x.to_string() << " by " << n;
    }
}

TEST_P(IntBigTBitwiseShifts, NegSmallByPosVarious)
{
    ASSERT_NO_FATAL_FAILURE(
            assertAll_paired(
                    TestData::small_negative, TestData::positive_shifts
            )
    );
}

TEST_P(IntBigTBitwiseShifts, NegLargeByPosVarious)
{
    ASSERT_NO_FATAL_FAILURE(
            assertAll_paired(
                    TestData::large_negative, TestData::positive_shifts
            )
    );
}

class IntBigTBitwiseCopyingShifts : public ::testing::TestWithParam<ShiftDir>
{
protected:
    void assert_likeReference(const std::string& s, int n)
    {
        intbig_t x = intbig_t::from_decimal(s);
        InfInt y = s;

        if(GetParam() == ShiftDir::LEFT) {
            for(int i = 0; i < n; i++) {
                y *= 2;
            }
        }
        else if(GetParam() == ShiftDir::RIGHT) {
            for(int i = 0; i < n; i++) {
                if(y >= 0) {
                    y /= 2;
                }
                else {
                    y /= 2;

                    if(y == 0) {
                        y = "-1";
                    }
                }
            }
        }
        else {
            FAIL() << "Unknown direction: " << GetParam();
        }

        ASSERT_EQ(
                (GetParam() == ShiftDir::LEFT ? x << n : x >> n).to_string(),
                y.toString()
        ) << intbig_t::from_decimal(s).to_hex_chunks() << (GetParam() == ShiftDir::LEFT ? " << " : " >> ") << n;
    }

    void assertAll_paired(const std::vector<std::string>& xs, const std::vector<int>& ns)
    {
        for(const std::string& x : xs) {
            for(const int n : ns) {
                ASSERT_NO_FATAL_FAILURE(assert_likeReference(x, n));
            }
        }
    }
};

INSTANTIATE_TEST_CASE_P(LeftShift, IntBigTBitwiseCopyingShifts, ::testing::Values(ShiftDir::LEFT));

INSTANTIATE_TEST_CASE_P(RightShift, IntBigTBitwiseCopyingShifts, ::testing::Values(ShiftDir::RIGHT));

TEST_P(IntBigTBitwiseCopyingShifts, PosSmallByPosVarious)
{
    ASSERT_NO_FATAL_FAILURE(
            assertAll_paired(
                    TestData::small_positive, TestData::positive_shifts
            )
    );
}

TEST_P(IntBigTBitwiseCopyingShifts, PosLargeByPosVarious)
{
    ASSERT_NO_FATAL_FAILURE(
            assertAll_paired(
                    TestData::large_positive, TestData::positive_shifts
            )
    );
}
