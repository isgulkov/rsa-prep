
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
    friend std::ostream& operator<<(std::ostream& os, const intbig_t& value);

    bool operator==(const intbig_t& other) const;
    bool operator!=(const intbig_t& other) const;

private:
    static int compare_3way_unsigned(const std::vector<uint64_t>& a_chunks,
                                      const std::vector<uint64_t>& b_chunks);
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
    static void add2_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x);
    static void sub2_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x);

    // "Universal" in-place adds that should work when result points to the same vector as x, y, or both of them
    //
    // TODO: these are kinda difficult to write well, so the plan is get away with copying and add2/sub2 for now, and
    // TODO: after everything is correct and tested, switch to in-place where appropriate
//    static void add3_unsigned(std::vector<uint64_t>& result,
//                              const std::vector<uint64_t>& x,
//                              const std::vector<uint64_t>& y);
//
//    static void sub3_unsigned(std::vector<uint64_t>& result,
//                              const std::vector<uint64_t>& x,
//                              const std::vector<uint64_t>& y);

public:
    void operator+=(const intbig_t& other);
    void operator-=(const intbig_t& other);

    intbig_t operator+(const intbig_t& other) const;
    intbig_t operator-(const intbig_t& other) const;
};

#endif //RSA_PREP_INTBIG_T_H
