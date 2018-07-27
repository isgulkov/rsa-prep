
#ifndef RSA_PREP_LONGINT_H
#define RSA_PREP_LONGINT_H

#include <string>

class uint_long
{
    /**
     * Abs. of `len` represents size of `data`
     * Sign of `len` is that of the number
     * `len` equal to zero represents zero
     */
    int32_t len = 0;
    uint32_t* data = nullptr;

public:
    /**
     * Construct a zero.
     */
    uint_long() = default;

    /**
     * Construct a number from its decimal string representation.
     */
    explicit uint_long(const std::string& s);

    /**
     * Construct a number from an unsigned integer and a sign.
     */
    explicit uint_long(uint64_t x, bool neg);

    /**
     * Construct a number from a 64-bit signed integer.
     */
    explicit uint_long(int64_t x);

    bool operator==(const uint_long& other) const;
    bool operator!=(const uint_long& other) const;

};

#endif //RSA_PREP_LONGINT_H
