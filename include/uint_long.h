
#ifndef RSA_PREP_LONGINT_H
#define RSA_PREP_LONGINT_H

#include <string>

class uint_long
{
    bool is_negative = false;

    // Let zero always be represented with 0/nullptr, and not 1/{0}?
    size_t len = 0;
    uint32_t* data = nullptr;

    // TODO: benchmark against fixed-length stack-located one:
    // TODO: uint_long<size_t Length> { /*...*/ uint32_t data[Length]; /*...*/ }
    // TODO: (Length would be 32 to 128 for RSA keys which are 1024 to 4096 bits)
    // TODO: (that seems kinda large for the stack, and anything less wouldn't really matter, right?)

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

    // TODO: +=
    // TODO: to_string
};

#endif //RSA_PREP_LONGINT_H
