
#ifndef RSA_PREP_LONGINT_H
#define RSA_PREP_LONGINT_H

#include <string>

#include "InfInt.h"

class uint_long
{
    /**
     * Abs. of `len` represents size of `data`
     * Sign of `len` is that of the number
     * `len` equal to zero represents zero
     */
    int32_t len = 0;
    uint32_t* data = nullptr;

    size_t sz_data() const
    {
        // REMOVE: there's got to be a better way!
        return (size_t)std::abs(len);
    }

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

private:
    /**
     * Three-way comparison that provides common implementation for operators <code>&lt;</code>, <code>&lt;=</code>,
     * <code>></code> and <code>>=</code>.
     */
    int compare(const uint_long& other) const;

public:
    bool operator <(const uint_long& other) const;
    bool operator<=(const uint_long& other) const;
    bool operator>=(const uint_long& other) const;
    bool operator >(const uint_long& other) const;

private:
    void resize_data(size_t new_size);

public:
    void operator+=(const uint_long& other);
    void operator-=(const uint_long& other);

    // REMOVE: temporary, until other conversions are implemented
    std::string to_string()
    {
        InfInt x;

        for(size_t i = 0; i < std::abs(len); i++) {
            x *= std::to_string((uint64_t)UINT32_MAX + 1);
            x += data[i];
        }

        return x.toString();
    }
};

#endif //RSA_PREP_LONGINT_H
