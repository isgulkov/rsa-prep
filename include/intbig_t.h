
#ifndef RSA_PREP_INTBIG_T_H
#define RSA_PREP_INTBIG_T_H

#include <vector>
#include <string>

class intbig_t
{
    /**
     * Internal representation:
     *
     *   - `is_neg`: the number's sign
     *       - `true` for negative numbers;
     *       - `false` otherwise (including 0).
     *
     *   - `chunks`: the number's absolute value
     *       - as 64-bit digits (base 2^64);
     *       - little-endian (least to most significant);
     *       - with no leading zeroes (thus, empty for 0).
     */

    bool is_neg = false;
    std::vector<uint64_t> chunks;

public:
    /**
     * Construct a zero
     */
    intbig_t() = default;

private:
    intbig_t(bool is_neg, std::vector<uint64_t>&& chunks);

public:
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    intbig_t(int64_t x);

    static intbig_t from_decimal(const std::string& s);

    std::string to_string() const;
    friend std::ostream& operator<<(std::ostream& os, const intbig_t& value);

    bool operator==(const intbig_t& other) const;
    bool operator!=(const intbig_t& other) const;

private:
    int compare_3way(const intbig_t& other) const;

public:
    bool operator <(const intbig_t& other) const;
    bool operator<=(const intbig_t& other) const;
    bool operator>=(const intbig_t& other) const;
    bool operator >(const intbig_t& other) const;

public:
    void operator+=(const intbig_t& other);
    void operator-=(const intbig_t& other);
};

#endif //RSA_PREP_INTBIG_T_H
