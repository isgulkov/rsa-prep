#include <fstream>

#include "gtest/gtest.h"

#include "sha256.h"

struct sha256_test {
    std::string msg_bytes;
    std::string digest_hex;
};

void assert_token_eq(std::string value, std::istream& istream)
{
    std::string tok;

    istream >> tok;

    ASSERT_EQ(tok, value);
}

sha256_test read_sha256_fips180_vec(std::istream& f_vectors)
{
    assert_token_eq("Comment:", f_vectors);
    assert_token_eq("length", f_vectors);
    size_t bit_length;
    f_vectors >> bit_length;

    assert_token_eq("Message:", f_vectors);

    std::string msg_bytes;

    {
        std::string msg_hex;
        f_vectors >> msg_hex;

        for(size_t i_byte = 0; i_byte < bit_length / 8; i_byte++) {
            char byte = (char)stoi(msg_hex.substr(i_byte * 2, 2), 0, 16);

            msg_bytes += byte;
        }
    }

    assert_token_eq("Digest:", f_vectors);
    std::string digest_hex;
    f_vectors >> digest_hex;

    assert_token_eq("Test:", f_vectors);
    assert_token_eq("Verify", f_vectors);

    return { msg_bytes, digest_hex };
}

std::vector<sha256_test> read_all_sha256_fips180_vecs()
{
    std::ifstream f_vectors("../test/vectors/sha2_256_fips_180.txt");

    {
        std::string tok;

        do {
            f_vectors >> tok;
        } while(tok != "SHA-256");
    }

    std::vector<sha256_test> all_tests;

    while(!f_vectors.eof()) {
        all_tests.push_back(read_sha256_fips180_vec(f_vectors));
    }

    EXPECT_EQ(all_tests.size(), 129) << "The FIPS 180 file is supposed to have exactly 129 tests in it.";

    return all_tests;
}

class SHA256Verify : public testing::TestWithParam<sha256_test> {};

TEST_P(SHA256Verify, VerifyDigest) {
    const std::string& msg_bytes = GetParam().msg_bytes;
    const std::string& digest_hex = GetParam().digest_hex;

    ASSERT_EQ(SHA256::sha256_hex(msg_bytes), digest_hex)
                                << "Message was " << msg_bytes.size() << " bytes long: "
                                << (msg_bytes.size() <= 40 ? msg_bytes : (msg_bytes.substr(0, 40) + "..."));
}

INSTANTIATE_TEST_CASE_P(FIPS180Vectors, SHA256Verify,
                        ::testing::ValuesIn(read_all_sha256_fips180_vecs())
);

INSTANTIATE_TEST_CASE_P(OtherVectors, SHA256Verify,
                        ::testing::Values(
                                sha256_test {
                                    std::string(10, '\0'),
                                    "01d448afd928065458cf670b60f5a594d735af0172c8d67f22a81680132681ca"
                                },
                                sha256_test {
                                        "hui",
                                        "a7877a970f44a733e53af458a618824987373f0a48d0ef36539d7f6d0c6a504e"
                                },
                                sha256_test {
                                        "\xf0\x9f\x8c\x9d",
                                        "e54d75effd2f6c5b303ab1428d240d1ed7581dbbcfc2bc314e9a6433e604ac60"
                                }
                        )
);
