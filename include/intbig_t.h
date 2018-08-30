
#ifndef RSA_PREP_INTBIG_T_H
#define RSA_PREP_INTBIG_T_H

#include <vector>
#include <string>
#include <functional>

#include <iostream> // For the stream i/o methods

class intbig_t
{
    /**
     * Internal representation:
     *
     *   - `sign`: the number's sign:
     *       - -1 -- negative;
     *       -  1 -- positive;
     *       -  0 -- zero.
     *
     *   - `chunks`: the number's absolute value
     *       - in base 2^64, so with 64-bit integers as digits
     *           (the term "chunk" is used to avoid confusion with decimal digits 0-9);
     *       - little-endian (least to most significant);
     *       - with no leading zeroes (thus, empty for 0).
     */

    // REMOVE:   !!!!!!!!!!
public:
    // REMOVE: / !!!!!!!!!!
    int sign = 0;
    std::vector<uint64_t> chunks;

public:
    /**
     * Construct a zero
     */
    intbig_t() = default;

    // Rule of zero applies, so all 5 defaults are implicitly defined:
//    intbig_t(const intbig_t& other) = default;
//    intbig_t(intbig_t&& other) = default;
//
//    intbig_t& operator=(intbig_t&& other) = default;
//    intbig_t& operator=(const intbig_t& other) = default;
//
//    ~intbig_t() = default;

private:
    intbig_t(int sign, std::vector<uint64_t>&& chunks);

public:
    // TODO: Replace this:
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    intbig_t(int64_t x);

    // TODO: with this:
    static intbig_t of(const int64_t x);
    // TODO: Also, replace this
    static intbig_t from_decimal(const std::string& decimal);
    // TODO: with this:
    static intbig_t from(const std::string& s, int base = 10);

    std::string to_string(int base = 10) const;
    // TODO: much better names for these
    std::string to_hex_chunks() const;
    std::string to_bin_chunks() const;
    // TODO: / (showing number as its chunks)

    friend std::istream& operator>>(std::istream&, intbig_t& value);
    friend std::ostream& operator<<(std::ostream& os, const intbig_t& value);

    // NOTE: PKCS#1 conversions
    static intbig_t from_bytes(std::string& bytes);
    static intbig_t from_bytes(std::istream& stream);

    size_t num_bytes() const;
    std::string to_bytes();
    void to_bytes(std::ostream& stream);
    // NOTE: / PKCS#1 conversions

    // TODO: if ever useful:
    size_t num_bits() const;

    //
    bool operator==(const intbig_t& other) const;
    bool operator!=(const intbig_t& other) const;

private:
    int compare_3way_unsigned(const intbig_t& other) const;
    int compare_3way(const intbig_t& other) const;

public:
    bool operator <(const intbig_t& other) const;
    bool operator<=(const intbig_t& other) const;
    bool operator>=(const intbig_t& other) const;
    bool operator >(const intbig_t& other) const;

    //
    intbig_t operator+() const;
    intbig_t operator-() const;
    intbig_t& negate();

private:
    // Set this number to zero (for when two equal numbers are subtracted)
    intbig_t& clear();

    /*
     * Apply operations just to the absolute values of the two numbers (in-place, with signed result)
     */

    // this = |this| + |other|
    void add_abs(const intbig_t& other);
    // this = |this| - |other|
    void sub_abs(const intbig_t& other);
    // this = |other| - |this|
    void subfrom_abs(const intbig_t& other);

public:
    intbig_t& operator+=(const intbig_t& other);
    intbig_t& operator-=(const intbig_t& other);

    intbig_t operator+(const intbig_t& other) const;
    intbig_t operator-(const intbig_t& other) const;

private:
    void inc_abs();
    void dec_abs();

public:
    intbig_t& operator++();
    intbig_t& operator--();

    // REVIEW: Should these really return const? clang-Tidy says so, but the original commit where they add this check
    // REVIEW: references a dead link on some sketchy CMU site. Not on web.archive.org, nowhere on Google.
    const intbig_t operator++(int);
    const intbig_t operator--(int);

    //
private:
    intbig_t& apply_bitwise(const intbig_t& other, const std::function<uint64_t(uint64_t, uint64_t)>& f_bitwise);

public:
    intbig_t& operator&=(const intbig_t& other);
    intbig_t& operator|=(const intbig_t& other);
    intbig_t& operator^=(const intbig_t& other);
    intbig_t& operator<<=(int64_t n);
    intbig_t& operator>>=(int64_t n);

    intbig_t operator&(const intbig_t& other) const;
    intbig_t operator|(const intbig_t& other) const;
    intbig_t operator^(const intbig_t& other) const;
    intbig_t operator<<(int64_t n) const;
    intbig_t operator>>(int64_t n) const;

    intbig_t operator~() const;

    //
    intbig_t& operator*=(const intbig_t& other);
    intbig_t operator*(const intbig_t& other) const;

    intbig_t& operator%=(const intbig_t& other);
    intbig_t& operator/=(const intbig_t& other);

    intbig_t operator%(const intbig_t& other) const;
    intbig_t operator/(const intbig_t& other) const;
//    std::pair<intbig_t, intbig_t> divmod(const intbig_t& other) const;

    intbig_t& to_power(int64_t p);
    intbig_t power(int64_t p) const;
};

#endif //RSA_PREP_INTBIG_T_H
