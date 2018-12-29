
#include "intbig_t.h"

#include <stdexcept>
#include <algorithm>

// REMOVE: temporary, for the following:
// REMOVE:  - to_string
// REMOVE:  - from_decimal
#include "InfInt.h"

/*
 * TODO: decide how much to reserve
 *       (i.e. how much bytes will we need and how much to pass to "reverse" to achieve that amount)
 *
 * TODO: use a common a private method for reservation?
 * TODO: check if vectors are copied and moved with their reservations (pretty sure, they are)
 * TODO: if so, only reserve when creating new objects, not in copies or moves
 *
 * Note: reserve() on an empty vector (which has no buffer) causes it to allocate its buffer right away
 */
constexpr size_t INITIAL_RESERVATION = 20;

intbig_t::intbig_t(int sign, std::vector<uint64_t>&& limbs) : sign(sign), limbs(std::move(limbs))
{
    limbs.reserve(INITIAL_RESERVATION);
}

namespace
{
int sign_of(int64_t x)
{
    if(x < 0) {
        return -1;
    }
    else if(x == 0) {
        return 0;
    }
    else {
        return 1;
    }
}
}

intbig_t::intbig_t(int64_t x) : sign(sign_of(x))
{
    limbs.reserve(INITIAL_RESERVATION);

    // Respect the representation of zero with empty vector
    if(x != 0) {
        limbs = { (uint64_t)(x < 0 ? -x : x) };
    }
}

intbig_t intbig_t::of(const int64_t x)
{
    return intbig_t(x);
}

intbig_t intbig_t::from_decimal(const std::string& decimal)
{
    // REMOVE: temporary, until proper string conversions are implemented

    InfInt fifth_leg(decimal);

    InfInt TWO_64 = InfInt(UINT64_MAX) + 1;

    int sign = fifth_leg < 0 ? -1 : (fifth_leg == 0 ? 0 : 1);
    std::vector<uint64_t> limbs;

    if(fifth_leg < 0) {
        fifth_leg *= -1;
    }

    while(fifth_leg != 0) {
        limbs.push_back((fifth_leg % TWO_64).toUnsignedLongLong());

        fifth_leg /= TWO_64;
    }

    return { sign, std::move(limbs) };
}

size_t intbig_t::size() const
{
    return limbs.size();
}

size_t intbig_t::num_bits() const
{
    size_t num_bits_last = 0;

    for(uint64_t last = limbs.back(); last; last >>= 1) {
        num_bits_last += 1;
    }

    return 64 * limbs.size() + num_bits_last;
}

std::string intbig_t::to_string(int base) const
{
    // REMOVE: temporary, until proper string conversions are implemented

    // REVIEW: yikes!
    const uint64_t HALF_LIMB = 1ULL << 32U;

    InfInt x;

    for(size_t i_back = 0; i_back < limbs.size(); i_back++) {
        x *= InfInt(HALF_LIMB);
        x += limbs[(limbs.size() - 1) - i_back] >> 32U;

        x *= InfInt(HALF_LIMB);
        x += limbs[(limbs.size() - 1) - i_back] & (HALF_LIMB - 1);
    }

    if(sign < 0) {
        x *= -1;
    }

    return x.toString();
}

namespace
{

std::string uint64_as_hex(uint64_t x)
{
    std::string result(64 / 4, '0');

    for(ssize_t i = 64 / 4 - 1; x; i--) {
        auto x_digit = uint8_t(x & 0xFU);

        x >>= 4;

        if(x_digit < 10) {
            result[i] = '0' + x_digit;
        }
        else {
            result[i] = (char)('A' - 10) + x_digit;
        }
    }

    return result;
}

}

std::string intbig_t::to_hex_chunks() const
{
    // REMOVE: dev version -- make more usable and possibly roundtrip-convertible

    if(limbs.empty()) {
        return "  " + uint64_as_hex(0);
    }

    std::string result = sign < 0 ? "-" : " ";

    for(ssize_t i = limbs.size() - 1; i >= 0; i--) {
        result += " " + uint64_as_hex(limbs[i]);
    }

    return result;
}

std::ostream& operator<<(std::ostream& os, const intbig_t& value)
{
    return os << value.to_string();
}

bool intbig_t::operator==(const intbig_t& other) const
{
    return sign == other.sign && limbs == other.limbs;
}

bool intbig_t::operator!=(const intbig_t& other) const
{
    return sign != other.sign || limbs != other.limbs;
}

int intbig_t::compare_3way_unsigned(const intbig_t& other) const
{
    // No leading zeroes are allowed, so longer value is necessarily larger
    if(limbs.size() < other.limbs.size()) {
        return -1;
    }
    else if(limbs.size() > other.limbs.size()) {
        return 1;
    }

    // From most significant to least significant
    for(ssize_t i = limbs.size() - 1; i >= 0; i--) {
        /*
         * Can't simply return a - b here -- it may not fit neither int nor int64_t
         */

        // REVIEW: const uint64_t&
        // REVIEW: a way to make their signed difference fit some type?
        const uint64_t our_limb = limbs[i];
        const uint64_t their_limb = other.limbs[i];

        if(our_limb < their_limb) {
            return -1;
        }
        else if(our_limb > their_limb) {
            return 1;
        }
    }

    return 0;
}

int intbig_t::compare_3way(const intbig_t& other) const
{
    // Since C++20, there's <=>

    if(sign != other.sign) {
        /*
         * -1 -  1 --> -2
         * -1 -  0 --> -1
         *  0 -  1 --> -1
         */
        return sign - other.sign;
    }

    return sign * compare_3way_unsigned(other);
}

bool intbig_t::operator<(const intbig_t& other) const
{
    return compare_3way(other) < 0;
}

bool intbig_t::operator<=(const intbig_t& other) const
{
    return compare_3way(other) <= 0;
}

bool intbig_t::operator>=(const intbig_t& other) const
{
    return compare_3way(other) >= 0;
}

bool intbig_t::operator>(const intbig_t& other) const
{
    return compare_3way(other) > 0;
}

intbig_t intbig_t::operator+() const
{
    // Return a copy
    return intbig_t(*this);

    // I was 100% sure I couldn't do this here:
//    return *this;
    // but there's no difference!

    /*
     * In this expression just the copy constructor is called:
     *     intbig_t y = +x;
     * And is this one -- copy then move:
     *     intbig_t y = std::move(+x);
     * In both cases `y` ends up with a copy like intended.
     *
     * All right. When return is used with a prvalue expression, like here:
     *     return intbig_t(*this);
     * RVO takes place, and `y` is directly initialized by this expression; so only thing that happens is the evident
     * copy.
     *
     * Now, this function's return type is non-reference, so when it is used with an lvalue expression:
     *     return *this;
     * There happens an implicit lvalue-to-rvalue conversion, which "effectively copy-constructs" a prvalue temporary.
     * This apparently happens "before" the actual return, so return value optimization happens just like in the
     * previous case.
     *
     * So the only difference between the two is explicit copy versus implicit. Phew. Better stick with explicit one.
     *
     * https://en.cppreference.com/w/cpp/language/return#Notes
     * https://en.cppreference.com/w/cpp/language/copy_elision
     * https://en.cppreference.com/w/cpp/language/implicit_conversion#Lvalue_to_rvalue_conversion
     * https://en.cppreference.com/w/cpp/language/value_category#lvalue
     * https://en.cppreference.com/w/cpp/language/value_category#prvalue
     */
}

intbig_t intbig_t::operator-() const
{
    return intbig_t(
            -sign,  // Preserve false for zero
            std::vector<uint64_t>(limbs)  // Explicitly copy the vector for the && parameter
    );
}

intbig_t& intbig_t::negate()
{
    sign = -sign;

    return *this;
}

namespace {
    void add2_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x)
    {
        /*
         * In-place add the two unsigned big integers:
         *   acc = acc + x
         */

        if(acc.size() < x.size()) {
            acc.resize(x.size());
        }

        bool carry = false;

        for(size_t i = 0; i < x.size(); i++) {
            // In case acc and x is actually the same vector
            // REVIEW: either find a way to avoid this copy or apply the hack to subs as well
            const uint64_t x_limb = x[i];

            acc[i] += x[i] + carry;

            // Set carry if overflow occured
            if(!carry) {
                carry = acc[i] < x_limb;
            }
            else {
                carry = acc[i] <= x_limb;
            }
        }

        // Propagate the carry, if any, through the acc's higher digits
        for(size_t i = x.size(); carry && i < acc.size(); i++) {
            acc[i] += carry;

            carry = (acc[i] == 0);
        }

        // potentially reaching a power of 64
        if(carry) {
            acc.push_back(1);
        }
    }

    void sub2_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x)
    {
        /*
         * In-place subtract the two unsigned big integers:
         *   acc = acc - x
         *
         * Pre: acc >= x
         *
         * NOTE: while the two versions of this method look like they can be easily refactored into one, they actually
         * have very little in common, so the result would consist completely of ifs.
         */

#ifndef NDEBUG
        if(acc.size() < x.size()) {
            std::logic_error("Negative result -- this should't happen");
        }
#endif

        bool carry = false;

        for(size_t i = 0; i < x.size(); i++) {
            uint64_t acc_limb = acc[i] - carry - x[i];

            // Set carry if overflow has occurred
            if(!carry) {
                carry = acc_limb > acc[i];
            }
            else {
                carry = acc_limb >= acc[i];
            }

            acc[i] = acc_limb;
        }

        // Collect the remaining carry, if any
        for(size_t i = x.size(); carry; i++) {
#ifndef NDEBUG
            if(i == acc.size()) {
                break;
            }
#endif
            acc[i] -= carry;

            carry = (acc[i] == UINT64_MAX);
        }

#ifndef NDEBUG
        if(carry) {
            std::logic_error("Runaway carry -- this should't happen");
        }
#endif

        // REVIEW: can't this be done better (maintaining a variable through both loops doesn't seem like it)
        while(acc.back() == 0) {
            acc.pop_back();
        }
    }

    void sub2from_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x)
    {
        /*
         * In-place subtract the two unsigned big integers:
         *   acc = x - acc
         *
         * Pre: x >= acc
         */

#ifndef NDEBUG
        if(x.size() < acc.size()) {
            std::logic_error("Negative result -- this should't happen");
        }
#endif

        bool carry = false;

        for(size_t i = 0; i < acc.size(); i++) {
            uint64_t limb = x[i] - acc[i] - carry;

            if(!carry) {
                carry = limb > x[i];
            }
            else {
                carry = limb >= x[i];
            }

            acc[i] = limb;
        }

        for(size_t i = acc.size(); i < x.size(); i++) {
            acc.push_back(x[i] - carry);

            carry = carry && (acc[i] == UINT64_MAX);
        }

#ifndef NDEBUG
        if(carry) {
            std::logic_error("Runaway carry -- this should't happen");
        }
#endif

        // REVIEW: see the other method
        while(acc.back() == 0) {
            acc.pop_back();
        }
    }
}

void intbig_t::add_abs(const intbig_t& other)
{
    /*
     * In-place add the absolute values:
     *   this = |this| + |other|
     *
     * Pre: both `this` and `other` are non-zero
     */

    add2_unsigned(limbs, other.limbs);
    sign = 1;
}

intbig_t& intbig_t::clear()
{
    limbs.resize(0);
    sign = 0;

    return *this;
}

void intbig_t::sub_abs(const intbig_t& other)
{
    /*
     * In-place subtract the absolute values:
     *   this = |this| - |other|
     *
     * Pre: both `this` and `other` are non-zero
     *
     * REVIEW: somehow merge these two methods into one?
     */

    int cmp_with_other = compare_3way_unsigned(other);

    if(cmp_with_other < 0) {
        sub2from_unsigned(limbs, other.limbs);
        sign = -1;
    }
    else if(cmp_with_other == 0) {
        // This is just an optimization -- the base case could handle this fine
        clear();
    }
    else {
        sub2_unsigned(limbs, other.limbs);
        sign = 1;
    }
}

void intbig_t::subfrom_abs(const intbig_t& other)
{
    /*
     * In-place subtract the absolute values:
     *   this = |other| - |this|
     *
     * Pre: both `this` and `other` are non-zero
     */

    int cmp_with_other = compare_3way_unsigned(other);

    if(cmp_with_other < 0) {
        sub2from_unsigned(limbs, other.limbs);
        sign = 1;
    }
    else if(cmp_with_other == 0) {
        clear();
    }
    else {
        sub2_unsigned(limbs, other.limbs);
        sign = -1;
    }
}

intbig_t& intbig_t::operator+=(const intbig_t& other)
{
    if(sign == -1) {
        if(other.sign == -1) {
            // -a + -b --> -(a + b)

            add_abs(other);
            negate();
        }
        else if(other.sign == 1) {
            // -a + b --> b - a

            subfrom_abs(other);
        }
    }
    else if(sign == 0) {
        if(other.sign != 0) {
            add_abs(other);
            sign = other.sign;
        }
    }
    else {
        if(other.sign == -1) {
            // a + -b --> a - b

            sub_abs(other);
        }
        else if(other.sign == 1) {
            // The base case:
            // a + b

            add_abs(other);
        }
    }

    return *this;
}

intbig_t& intbig_t::operator-=(const intbig_t& other)
{
    if(sign == -1) {
        if(other.sign == -1) {
            // -a - -b --> b - a

            subfrom_abs(other);
        }
        else if(other.sign == 1) {
            // -a - b --> -(a + b)

            add_abs(other);
            negate();
        }
    }
    else if(sign == 0) {
        if(other.sign != 0) {
            add_abs(other);
            sign = -other.sign;
        }
    }
    else {
        if(other.sign == -1) {
            // a - -b --> a + b

            add_abs(other);
        }
        else if(other.sign == 1) {
            // The base case:
            // a - b

            sub_abs(other);
        }
    }

    return *this;
}

intbig_t intbig_t::operator+(const intbig_t& other) const
{
    intbig_t result = intbig_t(*this);
    result += other;

    return result;
}

intbig_t intbig_t::operator-(const intbig_t& other) const
{
    intbig_t result = intbig_t(*this);
    result -= other;

    return result;
}

void intbig_t::inc_abs()
{
    for(uint64_t& limb : limbs) {
        if(++limb != 0) {
            return;
        }
    }

    limbs.push_back(1);
}

void intbig_t::dec_abs()
{
    for(uint64_t& limb : limbs) {
        if(limb-- != 0) {
            break;
        }
    }

    if(!limbs.back()) {
        limbs.pop_back();
    }

    if(limbs.empty()) {
        sign = 0;
    }
}

intbig_t& intbig_t::operator++()
{
    if(sign == -1) {
        dec_abs();
    }
    else if(sign == 0) {
        limbs.push_back(1);
        sign = 1;
    }
    else {
        inc_abs();
    }

    return *this;
}

intbig_t& intbig_t::operator--()
{
    if(sign == -1) {
        inc_abs();
    }
    else if(sign == 0) {
        limbs.push_back(1);
        sign = -1;
    }
    else {
        dec_abs();
    }

    return *this;
}

const intbig_t intbig_t::operator++(int)
{
    intbig_t old_value = *this;

    operator++();

    return old_value;
}

const intbig_t intbig_t::operator--(int)
{
    intbig_t old_value = *this;

    operator--();

    return old_value;
}

intbig_t& intbig_t::operator<<=(const int64_t n)
{
    // TODO: move to docs:
    // NOTE: shift of a 2-comp. negative works like it's applied to its absolute, except that >> stops at -1 (...111111)

    // TODO: implement both shifts as private methods accepting uint64_t, let operators handle the neg. and zero cases.
    // TODO: or maybe abstract the two away into one method with `n`'s sign determining the shift direction?

    if(n < 0) {
        return operator>>=(-n);
    }
    else if(n == 0 || sign == 0) {
        // Either number is already zero or is being shifted by zero -- both are no-ops
        return *this;
    }

    // REVIEW: rename to avoid Vietnam flashbacks?
    const uint64_t n_whole_limbs = (uint64_t)n / 64;

    if(n_whole_limbs != 0) {
        /*
         * Add an `n_whole_limbs`-long section of zeroes to the back, then move it to the front:
         *   ABC...XYZ --> ABC...XYZ[000] --> [000]ABC...XYZ
         */
        limbs.resize(limbs.size() + n_whole_limbs);
        std::rotate(limbs.begin(), limbs.end() - n_whole_limbs, limbs.end());
    }

    const uint64_t this_n = (uint64_t)n % 64;

    if(this_n != 0) {
        const uint64_t other_n = 64 - this_n;

        if(limbs.back() >> other_n != 0) {
            limbs.push_back(0);
        }

        for(size_t i = limbs.size() - 1; i > n_whole_limbs; i--) {
            /*
             * Shift this limb while accepting the lower neighbour's highest bits "ascending up".
             */

            limbs[i] = (limbs[i] << this_n) | (limbs[i - 1] >> other_n);
        }

        limbs[n_whole_limbs] <<= this_n;
    }

    return *this;
}

intbig_t& intbig_t::operator>>=(const int64_t n)
{
    // TODO: likewise

    if(n < 0) {
        return operator<<=(-n);
    }
    else if(n == 0 || sign == 0) {
        return *this;
    }

    // 1. Shift whole limbs -- remove as many as needed from the least significant side
    const uint64_t n_whole_limbs = (uint64_t)n / 64;

    if(n_whole_limbs != 0) {
        if(n_whole_limbs >= limbs.size()) {
            /*
             * Right shifting by more limbs than there are, leaving the number at the right shift's stationary point:
             *   - for negative numbers -- -1 (binary '...11111111'),
             *   - for non-negative numbers -- 0 (binary '...00000000').
             */
            if(sign == -1) {
                // For negative numbers, the stationary point is
                // TODO: doesn't this erase the vector's reservation? If it does, do this in a way that doesn't.
                limbs = { 1 };
            }
            else if(sign == 1) {
                clear();
            }

            return *this; // TODO: <-- rearrange stuff so that there's only one of this statement
        }

        /*
         * Move an `n_whole_limbs`-long section from the front to the back, then cut it off:
         *   [ABC]DEF...XYZ --> DEF...XYZ[ABC] --> DEF...XYZ
         */
        std::rotate(limbs.begin(), limbs.begin() + n_whole_limbs, limbs.end());
        limbs.resize(limbs.size() - n_whole_limbs);
    }

    // 2. Shift stuff along limb borders
    uint64_t this_n = (uint64_t)n % 64;

    if(this_n != 0) {
        const uint64_t other_n = 64 - this_n;

        for(size_t i = 0; i < limbs.size() - 1; i++) {
            /*
             * Shift this limb while accepting the upper neighbour's lowest bits being "passed down".
             */

            limbs[i] = (limbs[i] >> this_n) | (limbs[i + 1] << other_n);
        }

        limbs.back() >>= this_n;

        // TODO: somehow restructure this if into a prettier sight
        if(limbs.back() == 0) {
            if(limbs.size() == 1) {
                if(sign == -1) {
                    limbs[0] = 1;
                }
                else {
                    limbs.pop_back();
                    sign = 0;
                }
            }
            else {
                limbs.pop_back();
            }
        }
    }

    return *this;
}

intbig_t intbig_t::operator<<(int64_t n) const
{
    intbig_t result = intbig_t(*this);
    result <<= n;

    return result;
}

intbig_t intbig_t::operator>>(int64_t n) const
{
    intbig_t result = intbig_t(*this);
    result >>= n;

    return result;
}

intbig_t& intbig_t::apply_bitwise(const intbig_t& other, const std::function<uint64_t(uint64_t, uint64_t)>& f_bitwise)
{
    bool neg_result = f_bitwise(sign == -1 ? 1 : 0, other.sign == -1 ? 1 : 0) != 0;

    // REVIEW: document what the fuck is going on in this if's condition; check if it works for all 16 operations.
    if((sign == 1 && other.sign == 1 && f_bitwise(3, 5) == 1) || (sign == -1 && other.sign == -1 && f_bitwise(3, 5) == 7)) {
        limbs.resize(std::min(limbs.size(), other.limbs.size()));
    }
    else {
        limbs.resize(std::max(limbs.size(), other.limbs.size()));
    }

    // The "1"s that need to be added for conversion of terms into 2's complement
    bool this_add = sign == -1, other_add = other.sign == -1;

    for(size_t i = 0; i < limbs.size(); i++) {
        // Handle `this` as 2's complement
        if(sign == -1 && (limbs[i] = ~limbs[i] + this_add)) {
            this_add = false;
        }

        // REVIEW: Cut a corner or two when this condition is false?
        uint64_t other_limb = i < other.limbs.size() ? other.limbs[i] : 0;

        // Handle `other` as 2's complement
        if(other.sign == -1 && (other_limb = ~other_limb + other_add)) {
            other_add = false;
        }

        limbs[i] = f_bitwise(limbs[i], other_limb);

        // In this case, `this` ends up 2's complement -- convert it back
        if(neg_result) {
            limbs[i] = ~limbs[i];
        }
    }

    if(neg_result) {
        // ...finish the conversion from 2's complement
        inc_abs();
        sign = -1;
    }
    else {
        sign = 1;
    }

    // TODO: Do this in one resize instead of this loop.
    while(!limbs.empty() && limbs.back() == 0) {
        limbs.pop_back();
    }

    if(limbs.empty()) {
        sign = 0;
    }

    return *this;
}

intbig_t& intbig_t::operator&=(const intbig_t& other)
{
    if(limbs.empty()) {
        return *this;
    }
    else if(other.limbs.empty()) {
        return clear();
    }

    return apply_bitwise(other, [](uint64_t x, uint64_t y) {
        return x & y;
    });
}

intbig_t& intbig_t::operator|=(const intbig_t& other)
{
    if(limbs.empty()) {
        return operator=(other);
    }
    else if(other.limbs.empty()) {
        return *this;
    }

    return apply_bitwise(other, [](uint64_t x, uint64_t y) {
        return x | y;
    });
}

intbig_t& intbig_t::operator^=(const intbig_t& other)
{
    if(limbs.empty()) {
        return operator=(other);
    }
    else if(other.limbs.empty()) {
        return *this;
    }

    return apply_bitwise(other, [](uint64_t x, uint64_t y) {
        return x ^ y;
    });
}

intbig_t intbig_t::operator&(const intbig_t& other) const
{
    intbig_t result = intbig_t(*this);
    result &= other;

    return result;
}

intbig_t intbig_t::operator|(const intbig_t& other) const
{
    intbig_t result = intbig_t(*this);
    result |= other;

    return result;
}

intbig_t intbig_t::operator^(const intbig_t& other) const
{
    intbig_t result = intbig_t(*this);
    result ^= other;

    return result;
}

intbig_t intbig_t::operator~() const
{
    /*
     * Negatives are 2's complement (i.e. handled by the other bit operators as such):
     *   -this = ~this + 1;
     * which allows us to express inverse through negative:
     *   ~this = -this - 1.
     */

    return --operator-();
}

intbig_t& intbig_t::operator*=(const intbig_t& other)
{
    // NOTE: This is equivalent to the O(n^2) schoolbook method
    // TODO: Implement Karatsuba instead

    intbig_t result;

    for(uint64_t limb : other.limbs) {
        for(size_t i = 0; i < 64; i++) {
            if((limb & 1U) != 0) {
                result += *this;
            }

            limb >>= 1U;

            operator<<=(1);
        }
    }

    return *this = result;
}
