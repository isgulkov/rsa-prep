#include <vector>

#include "gtest/gtest.h"

#include "base64.hpp"

const std::vector<std::pair<std::string, std::string>> test_vectors = {
        // RFC4648
        { "", "" },
        { "f", "Zg==" },
        { "fo", "Zm8=" },
        { "foo", "Zm9v" },
        { "foob", "Zm9vYg==" },
        { "fooba", "Zm9vYmE=" },
        { "foobar", "Zm9vYmFy" },

        // Wikipedia article
        { "pleasure.", "cGxlYXN1cmUu" },
        { "leasure.", "bGVhc3VyZS4=" },
        { "easure.", "ZWFzdXJlLg==" },
        { "asure.", "YXN1cmUu" },
        { "sure.", "c3VyZS4=" },

        { {'\0'}, "AA==" },
        { "\x01\x02", "AQI=" },
        { "\xFD\xFE\xFF", "/f7/" },
        { "\xD1\x85\xD1\x83\xD0\xB9 \xF0\x9F\x8C\x9D", "0YXRg9C5IPCfjJ0=" }
};

const std::vector<std::pair<std::string, std::string>> test_vectors_ws = {
        { " aA\n==", "h" },
        { "  a\rHU=\n", "hu" },
        { "   aHV\tp", "hui" },
        { "     U  HV\t0a\t\tW4g\r\nVi5W\r\nLg = =", "Putin V.V." }
};

class Base64Codec : public testing::TestWithParam<std::pair<std::string, std::string>> {};
class Base64Decode : public testing::TestWithParam<std::pair<std::string, std::string>> {};

TEST_P(Base64Codec, Encode) {
    EXPECT_EQ(base64::b64encode(GetParam().first), GetParam().second)
                                << GetParam().first;
}

TEST_P(Base64Codec, Decode) {
    EXPECT_EQ(base64::b64decode(GetParam().second), GetParam().first)
                                << GetParam().second;
}

INSTANTIATE_TEST_CASE_P(TestVectors, Base64Codec,
        ::testing::ValuesIn(test_vectors)
);

TEST_P(Base64Decode, IgnoresWhitespace) {
    ASSERT_EQ(base64::b64decode(GetParam().first), GetParam().second);
}

INSTANTIATE_TEST_CASE_P(TestVectorsWS, Base64Decode,
        ::testing::ValuesIn(test_vectors_ws)
);
