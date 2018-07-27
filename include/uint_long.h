
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
    explicit operator int32_t()
    {
        if(len == 0) {
            return 0U;
        }
        else if(len == 1 || len == -1) {
            return len * data[0];
        }

        throw std::logic_error("Can't convert to int -- too big");
    }
};

#endif //RSA_PREP_LONGINT_H
