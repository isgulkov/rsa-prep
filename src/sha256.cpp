
#include "sha256.h"

namespace SHA256 {

typedef std::array<uint32_t, 64> msg_block;

uint32_t rot_r(uint32_t x, uint8_t n)
{
    return x >> n | (x << (32U - n));
}

uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (~x & z);
}

uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t Sigma0(uint32_t x)
{
    return rot_r(x, 2) ^ rot_r(x, 13) ^ rot_r(x, 22);
}

uint32_t Sigma1(uint32_t x)
{
    return rot_r(x, 6) ^ rot_r(x, 11) ^ rot_r(x, 25);
}

uint32_t sigma0(uint32_t x)
{
    return rot_r(x, 7) ^ rot_r(x, 18) ^ (x >> 3U);
}

uint32_t sigma1(uint32_t x)
{
    return rot_r(x, 17) ^ rot_r(x, 19) ^ (x >> 10U);
}

uint32_t from_4bytes_be(const char* bytes)
{
    uint32_t result = 0;

    for(size_t i = 0; i < 4; i++) {
        result <<= 8;
        result += bytes[i] & 0xFFU;
    }

    return result;
}

msg_block decomposed_block(const char* block)
{
    msg_block b;

    for(size_t i = 0; i < 16; i++) {
        b[i] = from_4bytes_be(block + 4 * i);
    }

    for(size_t i = 16; i < 64; i++) {
        b[i] = sigma1(b[i - 2]) + b[i - 7] + sigma0(b[i - 15]) + b[i - 16];
    }

    return b;
}

void apply_transform(sha256_hash& hash, uint32_t k_i, uint32_t w_i)
{
    const uint32_t t1 = hash[7] + Sigma1(hash[4]) + ch(hash[4], hash[5], hash[6]) + k_i + w_i;
    const uint32_t t2 = Sigma0(hash[0]) + maj(hash[0], hash[1], hash[2]);

    hash[7] = hash[6];
    hash[6] = hash[5];
    hash[5] = hash[4];
    hash[4] = hash[3] + t1;
    hash[3] = hash[2];
    hash[2] = hash[1];
    hash[1] = hash[0];
    hash[0] = t1 + t2;
}

void apply_sha2_padding(std::string& msg)
{
    // Make the message's length a multiple of 64 bytes

    const size_t length = msg.size();

    uint64_t n_pad_bytes = 64 - length % 64;

    /*
     * The padding has to be long enough for:
     *   - the 1 byte for the 1-bit;
     *   - the 8 bytes for the message length.
     */
    if(n_pad_bytes < 9) {
        n_pad_bytes += 64;
    }

    msg.resize(length + n_pad_bytes);

    msg[length] = (char)0x80;

    // Append the bit length, big-endian
    for(size_t i = msg.size() - 1, b_length = length * 8; b_length; b_length >>= 8) {
        msg[i--] = (char)(b_length % 0xFF);
    }
}

const uint32_t ks[] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

sha256_hash sha256_words(std::string msg)
{
    // TODO: make these non-copying by only adding padding to the last block

    apply_sha2_padding(msg);

    sha256_hash hash = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    for(size_t i_block = 0; i_block < msg.size() / 64; i_block++) {
        msg_block b = decomposed_block(msg.c_str() + 64 * i_block);

        sha256_hash new_state = hash;

        for(size_t i = 0; i < 64; i++) {
            apply_transform(new_state, ks[i], b[i]);
        }

        for(size_t i = 0; i < 8; i++) {
            hash[i] += new_state[i];
        }
    }

    return hash;
}

std::string sha256_hex(std::string msg)
{
    sha256_hash hash = sha256_words(std::move(msg));

    char hex[65];

    for(size_t i = 0; i < 8; i++) {
        sprintf(hex + 8 * i, "%08x", hash[i]);
    }

    return std::string(hex);
}

}
