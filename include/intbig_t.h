
#ifndef RSA_PREP_INTBIG_T_H
#define RSA_PREP_INTBIG_T_H

#include <vector>
#include <string>
#include <functional>

#include <iostream> // For the stream i/o methods

// TODO: put everything in a namespace
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
     *   - `limbs`: the number's absolute value
     *       - in base 2^64, so with 64-bit integers as digits
     *           (to avoid confusion with decimal digits 0-9, the term "limb" is used);
     *       - little-endian (least to most significant);
     *       - with no leading zeroes (thus, empty for 0).
     */

    // REMOVE:   !!!!!!!!!!
public:
    // REMOVE: / !!!!!!!!!!
    int sign = 0;
    std::vector<uint64_t> limbs;

public:
    /**
     * Construct the number zero
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
    intbig_t(int sign, std::vector<uint64_t>&& limbs);
    intbig_t(int sign, const std::vector<uint64_t>& limbs);

public:
    // REMOVE: replace this:
    // NOLINTNEXTLINE(google-explicit-constructor, hicpp-explicit-conversions)
    intbig_t(int64_t x);
    // TODO: with these:
    static intbig_t of(int64_t x);
//    static intbig_t of(uint64_t x);
    // TODO: if there's a conflict with shorter argument types, remove the second one or replace with the following:
//    static intbig_t of(uint64_t x, bool neg);

    intbig_t& operator=(int64_t x);

    enum Base { Binary = 2, Decimal = 10, Hex = 16, Base64 = 64, Base256 = 256 };

    static intbig_t from(const std::string& s, Base base = Decimal);

    // REMOVE: replace this:
    std::string to_hex_chunks() const;
    // TODO: with these:
    std::string to_string(Base base = Decimal) const;
//    std::string to_chunky_string(const Base base = Hex, const size_t zfill = 0) const;

    // TODO: Add implicit conversion to bool -- regular ints have it, so why not?

    friend std::istream& operator>>(std::istream&, intbig_t& value);
    friend std::ostream& operator<<(std::ostream& os, const intbig_t& value);

    size_t size() const; // TODO: is this useful?
    size_t num_bits() const; // TODO: what is this for 2-comp. negatives?

    // TODO: Test against consecutive divisions
    uint64_t factor2() const;

    // TODO: Find a way to test the random number constructors
    static intbig_t random_bits(size_t n_bits);
    static intbig_t random_lte(const intbig_t& x_max);

    // TODO: PKCS#1 conversions
    static intbig_t from_bytes(std::string& bytes);
    static intbig_t from_bytes(std::istream& stream);

    size_t num_bytes() const;
    // NOTE: `num_bytes` is intended as replacement for PKCS#1's xLen parameter, so must return exactly the number of
    // NOTE: bytes each of following two will produce

    std::string as_bytes();
    void as_bytes(std::ostream& stream);
    // TODO: / PKCS#1 conversions

    bool operator==(int64_t x) const;
    bool operator!=(int64_t x) const;

    bool operator==(const intbig_t& other) const;
    bool operator!=(const intbig_t& other) const;

private:
    int compare_3way_unsigned(uint64_t x) const;
    int compare_3way(int64_t x) const;

public:
    bool operator <(int64_t x) const;
    bool operator<=(int64_t x) const;
    bool operator>=(int64_t x) const;
    bool operator >(int64_t x) const;

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

    intbig_t& operator+=(int64_t x);
    intbig_t& operator-=(int64_t x);

    intbig_t operator+(int64_t x) const;
    intbig_t operator-(int64_t x) const;

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

    // These return const because of a clang-Tidy check based on some strange, never-to-be-found source:
    // https://clang.llvm.org/extra/clang-tidy/checks/cert-dcl21-cpp.html
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

    bool test_bit(size_t) const;

    uint64_t divmod(uint64_t);

    intbig_t& operator*=(int64_t);
    intbig_t& operator/=(int64_t);
    intbig_t& operator%=(int64_t);

    intbig_t operator*(int64_t) const;
    intbig_t operator/(int64_t) const;
    int64_t operator%(int64_t) const;

    intbig_t divmod(const intbig_t& other);

    intbig_t& operator*=(const intbig_t& other);
    intbig_t& operator/=(const intbig_t& other);
    intbig_t& operator%=(const intbig_t& other);

    intbig_t operator*(const intbig_t& other) const;
    intbig_t operator/(const intbig_t& other) const;
    intbig_t operator%(const intbig_t& other) const;

    intbig_t& square();

    intbig_t& to_power(const intbig_t& pow);
    intbig_t  at_power(const intbig_t& pow) const;

    /**
     * TODO: Reimplement both modular product and modular power through Montgomery reduction.
     */

    intbig_t& mul_mod(const intbig_t& other, const intbig_t& m);
    intbig_t  times_mod(const intbig_t& other, const intbig_t& m) const;

    intbig_t& to_power(const intbig_t& pow, const intbig_t& m);
    intbig_t  at_power(const intbig_t& pow, const intbig_t& m) const;

    intbig_t inverse_mod(const intbig_t& m) const;

    int64_t gcd(int64_t) const;
    intbig_t gcd(const intbig_t& other) const;
};

#endif //RSA_PREP_INTBIG_T_H
