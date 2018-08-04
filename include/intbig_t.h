
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
     *       - in base 2^64, so with 64-bit integers as digits
     *           (the term "chunk" is used to avoid confusion with decimal digits 0-9);
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

    // Rule of zero applies, so all 5 defaults are implicitly defined:
//    intbig_t(const intbig_t& other) = default;
//    intbig_t(intbig_t&& other) = default;
//
//    intbig_t& operator=(intbig_t&& other) = default;
//    intbig_t& operator=(const intbig_t& other) = default;
//
//    ~intbig_t() = default;

private:
    intbig_t(bool is_neg, std::vector<uint64_t>&& chunks);

public:
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    intbig_t(int64_t x);

    static intbig_t from_decimal(const std::string& decimal);

    std::string to_string() const;
    std::string to_hex() const;
    friend std::ostream& operator<<(std::ostream& os, const intbig_t& value);

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

    intbig_t operator+() const;
    intbig_t operator-() const;
    intbig_t& negate();

private:
    /*
     * Apply operations just on the absolute values of the numbers
     * This is done in-place (so, no temporaries), the result is stored in this
     *
     * NOTE: while only the absolute values are considered, the result may is signed
     */

    // Set this number to zero (for when two equal numbers are subtracted)
    // TODO: restore the explanation that the base case can handle it, but blah blah blah
    void clear();

    // this = |this| + |other|
    void add_abs(const intbig_t& other);
    // this = |this| - |other|
    void sub_abs(const intbig_t& other);
    // this = |other| - |this|
    void subfrom_abs(const intbig_t& other);

public:
    void operator+=(const intbig_t& other);
    void operator-=(const intbig_t& other);

    // TODO: Make the compounds return intbig_t&, and don't forget to:
    // TODO:   - employ it in the old tests (e.g. composables);
    // TODO:   - add (two) new tests for this behavior.

    intbig_t operator+(const intbig_t& other) const;
    intbig_t operator-(const intbig_t& other) const;

    /*
     * TODO:
     *   - refactor the shit out of the three "sub2" functions
     *   - change sign from bool to int (see doc)
     *   - it works!
     */
};

#endif //RSA_PREP_INTBIG_T_H
