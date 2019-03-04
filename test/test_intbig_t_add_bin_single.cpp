#include <vector>

#include "gtest/gtest.h"

extern "C" {
#include "mini-gmp.h"
}

#include "intbig_t.h"

namespace IntBigTAdditiveBinSingle
{

namespace TestData
{
const std::vector<std::tuple<std::string, int64_t, std::string>> operand_sign_cases = {
        { "14",   18,   "32" },
        { "140",  5,    "145" },
        { "10",   -15,  "-5" },
        { "15",   -10,  "5" },
        { "-10",  15,   "5" },
        { "-15",  10,   "-5" },
        { "-111", -333, "-444" },
        { "-33",  -11,  "-44" }
};

const std::vector<std::tuple<std::string, int64_t, std::string>> something_else = {
        { "14",   9,    "5" },
        { "5",    140,  "-135" },
        { "-10",  14,   "-24" },
        { "-159", -11,  "-148" },
        { "111",  -333, "444" }
};
}

TEST(IntBigTAdditive1, OperandSignCasesPlus) {
    for(const auto& t : TestData::operand_sign_cases) {
        EXPECT_EQ((intbig_t::from(std::get<0>(t)) + std::get<1>(t)).to_string(), std::get<2>(t))
            << std::get<0>(t) << " " << std::get<1>(t);
    }
}

TEST(IntBigTAdditive1, MinusGottaHaveSomethingToo) {
    for(const auto& t : TestData::something_else) {
        EXPECT_EQ((intbig_t::from(std::get<0>(t)) - std::get<1>(t)).to_string(), std::get<2>(t))
                            << std::get<0>(t) << " " << std::get<1>(t);
    }
}

// TODO: write better tests (once a better overall approach is found)
}
