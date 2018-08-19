
#ifndef RSA_PREP_SHA256_H
#define RSA_PREP_SHA256_H

#include <array>
#include <string>

namespace SHA256 {

typedef std::array<uint32_t, 8> sha256_hash;

sha256_hash sha256_words(const std::string& msg);
std::string sha256_hex(const std::string& msg);

}

#endif //RSA_PREP_SHA256_H
