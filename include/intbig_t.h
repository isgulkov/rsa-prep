
#ifndef RSA_PREP_INTBIG_T_H
#define RSA_PREP_INTBIG_T_H

#include <vector>
#include <string>

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
    // TODO: replace with `static of(int64_t x)`
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
    // Set this number to zero (for when two equal numbers are subtracted)
    void clear();

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
    void operator+=(const intbig_t& other);
    void operator-=(const intbig_t& other);

    intbig_t operator+(const intbig_t& other) const;
    intbig_t operator-(const intbig_t& other) const;

    /*
     * TODO:
     *   - change sign repr. from bool to int (see doc)
     *   - add subtraction shrinkage tests
     *   - optimize vector resize in subtraction (if possible)
     *   - refactor the two "sub2" functions and their method counterparts some more
     *   - make the compound assignments return lvalue reference to *this
     *     - add (two) tests for this
     *     - employ it in the old tests
     *   - it works!
     */
};

#endif //RSA_PREP_INTBIG_T_H
