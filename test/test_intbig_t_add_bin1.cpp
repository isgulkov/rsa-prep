#include <vector>

#include "gtest/gtest.h"

#include "intbig_t.h"

namespace IntBigTAdditiveBin1
{

namespace TestData
{
const std::vector<std::tuple<std::string, int64_t, std::string, std::string>> operand_sign_cases = {
        { "14",   18,   "32",   "-4" },
        { "140",  5,    "145",  "135" },
        { "10",   -15,  "-5",   "25" },
        { "15",   -10,  "5",    "25" },
        { "-10",  15,   "5",    "-25" },
        { "-15",  10,   "-5",   "-25" },
        { "-111", -333, "-444", "222" },
        { "-33",  -11,  "-44",  "-22" }
};

const std::vector<std::tuple<std::string, int64_t, std::string, std::string>> large_lhs = {
        {
            "3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312415963462",
            78787878,
            "3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312494751340",
            "3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312337175584"
        },
        {
            "1427247692705959881058285969449495136382748038",
            1000000000011LL,
            "1427247692705959881058285969449496136382748049",
            "1427247692705959881058285969449494136382748027"
        }
};
}

class IntBigTAdditive1 : public ::testing::TestWithParam<std::tuple<std::string, int64_t, std::string, std::string>>
{
protected:
    std::string GetX() { return std::get<0>(GetParam()); }
    int64_t GetY() { return std::get<1>(GetParam()); }
    std::string GetSum() { return std::get<2>(GetParam()); }
    std::string GetDiff() { return std::get<3>(GetParam()); }
};

TEST_P(IntBigTAdditive1, AddInplace1) {
    intbig_t x = intbig_t::from(GetX());
    x += GetY();
    ASSERT_EQ(x.to_string(), GetSum()) << GetX() << " " << GetY();
}

TEST_P(IntBigTAdditive1, AddCopying1) {
    ASSERT_EQ((intbig_t::from(GetX()) + GetY()).to_string(), GetSum()) << GetX() << " " << GetY();
}

TEST_P(IntBigTAdditive1, SubInplace1) {
    intbig_t x = intbig_t::from(GetX());
    x -= GetY();
    ASSERT_EQ(x.to_string(), GetDiff()) << GetX() << " " << GetY();
}

TEST_P(IntBigTAdditive1, SubCopying1) {
    ASSERT_EQ((intbig_t::from(GetX()) - GetY()).to_string(), GetDiff()) << GetX() << " " << GetY();
}

INSTANTIATE_TEST_CASE_P(SignCases, IntBigTAdditive1, ::testing::ValuesIn(TestData::operand_sign_cases));

INSTANTIATE_TEST_CASE_P(LargeLHS, IntBigTAdditive1, ::testing::ValuesIn(TestData::large_lhs));

}
