
#include "intbig_t.h"

#include <stdexcept>
#include <algorithm>
#include <sstream> // TODO!: remove?
#include <random>

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

intbig_t::intbig_t(int sign, std::vector<uint64_t>&& limbs) : sign(sign), limbs(limbs)
{
    this->limbs.reserve(INITIAL_RESERVATION);
}

intbig_t::intbig_t(int sign, const std::vector<uint64_t>& limbs) : sign(sign), limbs(limbs)
{
    this->limbs.reserve(INITIAL_RESERVATION);
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

intbig_t& intbig_t::operator=(const int64_t x)
{
    if((sign = sign_of(x))) {
        limbs = { (uint64_t)std::abs(x) };
    }
    else {
        limbs.clear();
    }

    return *this;
}

intbig_t intbig_t::from(const std::string& s, const Base base)
{
    if(base == Decimal) {
        auto it_c = s.begin();

        const bool is_neg = *it_c == '-';
        if(is_neg) ++it_c;

        intbig_t x;

        for(; it_c != s.end(); it_c++) {
            const auto dig = (uint8_t)(*it_c - '0');

            if(dig > 9) {
                throw std::invalid_argument(
                        "Invalid digit '" + std::string(1, *it_c) + "'"
                        + " at " + std::to_string(it_c - s.begin())
                        + " in \"" + s + "\""
                );
            }

            x *= 10;
            x += dig;
        }

        return is_neg ? x.negate() : x;
    }
    else if(base == Base256) {
        intbig_t x;

        for(char c : s) {
            x <<= 8;
            x += uint8_t(c);
        }

        return x;
    }
    else {
        throw std::logic_error("Base " + std::to_string(base) + " is not implemented");
    }
}

std::string intbig_t::to_string(const Base base) const
{
    if(base == Decimal) {
        if(!sign) {
            return "0";
        }

        // TODO: consider reducing the number of divisions by taking up to 18 digits at a time

        intbig_t value = *this;
        std::string s;

        while(value != 0) {
            s += (char)('0' + value.divmod(10));
        }

        if(sign < 0) {
            s += '-';
        }

        std::reverse(s.begin(), s.end());

        return s;
    }
    else if(base == Base256) {
        intbig_t x = *this;
        std::string s;

        while(x != 0) {
            s += char(x.limbs[0] % 256);
            x >>= 8;
        }

        std::reverse(s.begin(), s.end());

        return s;
    }
    else {
        throw std::logic_error("Base " + std::to_string(base) + " is not implemented");
    }
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

bool intbig_t::operator==(const int64_t x) const
{
    switch(limbs.size()) {
        case 0:
            return x == 0;
        case 1:
            if(x < 0) {
                return sign == -1 && limbs[0] == (uint64_t)-x;
            }
            else {
                return sign == 1 && limbs[0] == (uint64_t)x;
            }
        default:
            return false;
    }
}

bool intbig_t::operator!=(const int64_t x) const
{
    return !operator==(x);
}

bool intbig_t::operator==(const intbig_t& other) const
{
    return sign == other.sign && limbs == other.limbs;
}

size_t intbig_t::size() const
{
    return limbs.size();
}

size_t intbig_t::num_bits() const
{
    if(!sign) {
        return 0;
    }

    size_t num_bits_last = 0;

    for(uint64_t last = limbs.back(); last; last >>= 1) {
        num_bits_last += 1;
    }

    return 64 * (limbs.size() - 1) + num_bits_last;
}

uint64_t intbig_t::factor2() const
{
    uint64_t coef = 0;

    size_t i_limb = 0;

    for(; i_limb < limbs.size() && !limbs[i_limb]; i_limb++) {
        coef += 64;
    }

    if(i_limb < limbs.size()) {
        for(uint64_t limb = limbs[i_limb]; (limb & 1) == 0; limb >>= 1) {
            coef += 1;
        }
    }

    return coef;
}

std::vector<uint64_t> random_bits_upto(size_t n_bits)
{
    std::random_device rd;
    std::uniform_int_distribution<uint64_t> dist;

    std::vector<uint64_t> limbs;

    for(; n_bits >= 64; n_bits -= 64) {
        limbs.push_back(dist(rd));
    }

    if(n_bits) {
        uint64_t last_limb = dist(rd);

        last_limb &= (1ULL << n_bits) - 1;

        limbs.push_back(last_limb);
    }

    while(!limbs.empty() && !limbs.back()) {
        limbs.pop_back();
    }

    return limbs;
}

intbig_t intbig_t::random_bits(size_t n_bits)
{
    auto limbs = random_bits_upto(n_bits);

    if(n_bits) {
        // REVIEW: Why doesn't this work with (n_bits % 64 - 1)?..
        limbs.back() |= (1 << (n_bits % 64));
    }

    return { limbs.empty() ? 0 : 1, std::move(limbs) };
}

intbig_t intbig_t::random_lte(const intbig_t& x_max)
{
    if(x_max.sign < 1) {
        throw std::logic_error("Can only produce non-negative numbers less than a positive");
    }

    intbig_t x;

    do {
        // REVIEW: Get rid of this once these operations are implemented externally on naturals
        auto limbs = random_bits_upto(x_max.num_bits());

        x = { limbs.empty() ? 0 : 1, std::move(limbs) };
    } while(x_max < x);

    return x;
}

bool intbig_t::operator!=(const intbig_t& other) const
{
    return sign != other.sign || limbs != other.limbs;
}

int intbig_t::compare_3way_unsigned(uint64_t x) const
{
    switch(limbs.size()) {
        case 0:
            return -1 * (x != 0);
        case 1:
            if(limbs[0] < x) {
                return -1;
            }
            else if(limbs[0] == x) {
                return 0;
            }
            else {
                return 1;
            }
        default:
            return 1;
    }
}

int intbig_t::compare_3way(const int64_t x) const
{
    const int x_sign = sign_of(x);

    if(sign != x_sign) {
        return sign - x_sign;
    }

    return sign * compare_3way_unsigned((uint64_t)std::abs(x));
}

bool intbig_t::operator <(const int64_t x) const
{
    return compare_3way(x) < 0;
}

bool intbig_t::operator<=(const int64_t x) const
{
    return compare_3way(x) <= 0;
}

bool intbig_t::operator>=(const int64_t x) const
{
    return compare_3way(x) >= 0;
}

bool intbig_t::operator >(const int64_t x) const
{
    return compare_3way(x) > 0;
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
    void add2_unsigned(std::vector<uint64_t>& acc, uint64_t x)
    {
        for(size_t i = 0; x && i < acc.size(); i++) {
            const uint64_t new_limb = acc[i] + x;

            x = new_limb <= acc[i] ? 1 : 0;

            acc[i] = new_limb;
        }

        if(x) {
            acc.push_back(x);
        }
    }

    void sub2_unsigned(std::vector<uint64_t>& acc, uint64_t x)
    {
        for(size_t i = 0; x && i < acc.size(); i++) {
            const uint64_t new_limb = acc[i] - x;

            x = new_limb >= acc[i] ? 1 : 0;

            acc[i] = new_limb;
        }

        if(!acc.back()) {
            acc.pop_back();
        }
    }
}

intbig_t& intbig_t::operator+=(int64_t x)
{
    if(x < 0) {
        return operator-=(-x);
    }

    if(sign < 0) {
        // REVIEW: rewrite without the double negation? (here and in operator-=)

        negate();
        operator-=(x);
        negate();
    }
    else if(x != 0) {
        sign = 1; // For when this is zero

        add2_unsigned(limbs, (uint64_t)x);

        if(limbs.empty()) {
            sign = 0;
        }
    }

    return *this;
}

intbig_t& intbig_t::operator-=(const int64_t x)
{
    if(x < 0) {
        return operator+=(-x);
    }

    if(sign < 0) {
        negate();
        operator+=(x);
        negate();
    }
    else {
        auto _x = (uint64_t)x;

        if(limbs.size() == 1 && limbs[0] < _x) {
            std::swap(limbs[0], _x);
            negate();
        }

        sub2_unsigned(limbs, _x);

        if(limbs.empty()) {
            sign = 0;
        }
    }

    return *this;
}

intbig_t intbig_t::operator+(int64_t x) const
{
    return intbig_t(*this) += x;
}

intbig_t intbig_t::operator-(int64_t x) const
{
    return intbig_t(*this) -= x;
}

intbig_t& intbig_t::clear()
{
    limbs.resize(0);
    sign = 0;

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
            throw std::logic_error("Negative result -- this should't happen");
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
            throw std::logic_error("Runaway carry -- this should't happen");
        }
#endif

        // REVIEW: can't this be done better (maintaining a variable through both loops doesn't seem like it)
        while(!acc.empty() && acc.back() == 0) {
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
            throw std::logic_error("Negative result -- this should't happen");
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
            throw std::logic_error("Runaway carry -- this should't happen");
        }
#endif

        // REVIEW: see the other method
        while(!acc.empty() && acc.back() == 0) {
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
    return intbig_t(*this) += other;
}

intbig_t intbig_t::operator-(const intbig_t& other) const
{
    return intbig_t(*this) -= other;
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
    const intbig_t old_value = *this;

    operator++();

    return old_value;
}

const intbig_t intbig_t::operator--(int)
{
    const intbig_t old_value = *this;

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
    return intbig_t(*this) <<= n;
}

intbig_t intbig_t::operator>>(int64_t n) const
{
    return intbig_t(*this) >>= n;
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
    return intbig_t(*this) &= other;
}

intbig_t intbig_t::operator|(const intbig_t& other) const
{
    return intbig_t(*this) |= other;
}

intbig_t intbig_t::operator^(const intbig_t& other) const
{
    return intbig_t(*this) ^= other;
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

bool intbig_t::test_bit(const size_t i) const
{
    const size_t i_limb = i / 64;

    if(i_limb < limbs.size()) {
        return ((limbs[i_limb] >> (i % 64)) & 1) != 0;
    }
    else {
        return sign < 0;
    }
}

intbig_t& intbig_t::operator*=(const int64_t x)
{
    if(x < 0) {
        negate();
        return operator*=(-x);
    }
    else if(x == 0) {
        return clear();
    }
    else if(x == 1) {
        return *this;
    }

    uint64_t carry = 0;

    for(uint64_t& limb : limbs) {
        const uint64_t limb_low = limb & 0xFFFFFFFF;
        const uint64_t limb_high = limb >> 32;
        const uint64_t x_low = (uint64_t)x & 0xFFFFFFFF;
        const uint64_t x_high = (uint64_t)x >> 32;

        const uint64_t z0 = limb_low * x_low;

        uint64_t z1 = limb_high * x_low;
        const uint64_t z11 = limb_low * x_high;

        uint64_t z2 = limb_high * x_high;

        z1 += z0 >> 32;
        z1 += z11;

        if(z1 < z11) {
            z2 += 1ULL << 32;
        }

        limb = (z1 << 32) + (z0 & 0xFFFFFFFF) + carry;

        // TODO!: Invent a test vector where this fails without the (limb < carry) term.
        //  Also, can't this expression itself carry?
        carry = (limb < carry) + z2 + (z1 >> 32);
    }

    if(carry) {
        limbs.push_back(carry);
    }

    return *this;
}

uint64_t intbig_t::divmod(const uint64_t x)
{
    if(x == 0) {
        throw std::domain_error("Division by zero");
    }

    /**
     * TODO: implement this the way GMP does it:
     *  - https://gmplib.org/manual/Single-Limb-Division.html#Single-Limb-Division
     *  - https://gmplib.org/~tege/division-paper.pdf
     */

    uint64_t carry = 0;

    for(ssize_t i = limbs.size() - 1; i >= 0; i--) {
        uint64_t limb_high = (carry << 32) + (limbs[i] >> 32);

        carry = limb_high % x;
        limb_high /= x;

        uint64_t limb_low = (carry << 32) + (limbs[i] & 0xFFFFFFFF);
        carry = limb_low % x;
        limb_low /= x;

        limbs[i] = (limb_high << 32) | (limb_low & 0xFFFFFFFF);
    }

    if(!limbs.back()) {
        limbs.pop_back();

        if(limbs.empty()) {
            sign = 0;
        }
    }

    return carry;
}

intbig_t& intbig_t::operator/=(const int64_t x)
{
    if(x < 0) {
        negate();
        return operator/=(-x);
    }

    if(x != 1) {
        divmod((uint64_t)x);
    }

    return *this;
}


intbig_t& intbig_t::operator%=(const int64_t x)
{
    if(x < 0 || sign < 0) {
        throw std::logic_error("Not implemented yet");
    }

    return operator=(divmod((uint64_t)x));
}

intbig_t intbig_t::operator*(const int64_t x) const
{
    return intbig_t(*this) *= x;
}

intbig_t intbig_t::operator/(const int64_t x) const
{
    return intbig_t(*this) /= x;
}

int64_t intbig_t::operator%(const int64_t x) const
{
    if(x < 0 || sign < 0) {
        throw std::logic_error("Not implemented yet");
    }

    intbig_t x_div = *this;

    return x_div.divmod((uint64_t)x);
}

namespace
{
    /**
     * Perform "full word" multiplication on limbs.
     * TODO: use this in operator*=(uint64_t) as well
     *
     * @return Two-limb product of @code a and @code b
     */
    std::pair<uint64_t, uint64_t> mul_full(const uint64_t a, const uint64_t b)
    {
        const uint64_t a_low = a & 0xFFFFFFFF;
        const uint64_t a_high = a >> 32;
        const uint64_t b_low = b & 0xFFFFFFFF;
        const uint64_t b_high = b >> 32;

        const uint64_t z0 = a_low * b_low;

        uint64_t z1 = a_high * b_low;
        const uint64_t z11 = a_low * b_high;

        uint64_t z2 = a_high * b_high;

        z1 += z0 >> 32;
        z1 += z11;

        if(z1 < z11) {
            z2 += 1ULL << 32;
        }

        return { (z1 << 32) + (z0 & 0xFFFFFFFF), z2 + (z1 >> 32) };
    }

    void add1_at(std::vector<uint64_t>& acc, uint64_t x, const size_t i_radix)
    {
        if(acc.size() <= i_radix) {
            acc.resize(i_radix + 1);
        }

        for(size_t i = i_radix; x && i < acc.size(); i++) {
            acc[i] += x;

            x = acc[i] < x ? 1 : 0;
        }

        if(x) {
            acc.push_back(x);
        }
    }
}

intbig_t& intbig_t::operator*=(const intbig_t& other)
{
    if(other == 1) {
        return *this;
    }

    return operator=(operator*(other));
}

intbig_t intbig_t::operator*(const intbig_t& other) const
{
    // TODO: Karatsuba for numbers over a threshold (30 limbs in GMP)

    if(!sign || !other.sign) {
        return intbig_t();
    }
    else if(operator==(1)) {
        return other;
    }

    std::vector<uint64_t> new_limbs(limbs.size() + other.limbs.size() - 1);

    for(size_t i = 0; i < limbs.size(); i++) {
        for(size_t j = 0; j < other.limbs.size(); j++) {
            const auto prod = mul_full(limbs[i], other.limbs[j]);

            if(prod.first) {
                add1_at(new_limbs, prod.first, i + j);
            }

            if(prod.second) {
                add1_at(new_limbs, prod.second, i + j + 1);
            }
        }
    }

    return intbig_t(sign * other.sign, std::move(new_limbs));
}

intbig_t intbig_t::divmod(const intbig_t& other)
{
    if(!sign) {
        return intbig_t();
    }

    // TODO!: Handle negative signs and zeroes

    ssize_t n_bits_q = num_bits() - other.num_bits();

    if(n_bits_q < 0) {
        sign *= other.sign;

        intbig_t rem;
        std::swap(*this, rem);

        return rem;
    }

    const int sign_q = sign * other.sign;

    intbig_t denom = other << n_bits_q;

    std::vector<uint64_t> limbs_q = std::vector<uint64_t>(size_t(n_bits_q) / 64 + 1, 0);

    for(ssize_t i = n_bits_q; i >= 0; i--) {
        if(operator>=(denom)) {
            sub2_unsigned(limbs, denom.limbs);

            limbs_q[i / 64] |= 1ULL << (i % 64);
        }

        denom >>= 1;
    }

    if(limbs.empty()) {
        sign = 0;
    }

    /**
     * TODO: Use correct sign for the resulting remainder
     *  (while in principle remainder should always be non-negative, this may not apply to the one we get here)
     *
     * TODO: Sort out the exception that arises in ~std::vector if std::move(limbs_q) is used in this method (below
     *  and at the top)
     */
    intbig_t rem{ limbs_q.empty() ? 0 : sign_q, limbs_q };

    std::swap(rem, *this);

    return rem;
}

intbig_t& intbig_t::operator/=(const intbig_t& other)
{
    divmod(other);
    return *this;
}

intbig_t& intbig_t::operator%=(const intbig_t& other)
{
    return operator=(divmod(other));
}

intbig_t intbig_t::operator/(const intbig_t& other) const
{
    intbig_t x = *this;
    x.divmod(other);

    return x;
}

intbig_t intbig_t::operator%(const intbig_t& other) const
{
    intbig_t x = *this;

    return x.divmod(other);
}

intbig_t& intbig_t::square()
{
    if(sign == 0) {
        return *this;
    }
    else if(sign < 0) {
        negate();
    }

    if(operator==(1)) {
        return *this;
    }

    std::vector<uint64_t> new_limbs(2 * limbs.size() - 1);

    /**
     * Only multiply limbs i and j once, but add double the product to the result.
     */
    for(size_t i = 0; i < limbs.size(); i++) {
        for(size_t j = i; j < limbs.size(); j++) {
            auto prod = mul_full(limbs[i], limbs[j]);
            uint64_t prod_carry = 0;

            if(i != j) {
                prod_carry = prod.second >> 63;
                prod.second = (prod.second << 1) | (prod.first >> 63);
                prod.first <<= 1;
            }

            if(prod.first) {
                add1_at(new_limbs, prod.first, i + j);
            }

            if(prod.second) {
                add1_at(new_limbs, prod.second, i + j + 1);
            }

            if(prod_carry) {
                add1_at(new_limbs, 1, i + j + 2);
            }
        }
    }

    limbs = new_limbs;

    return *this;
}

intbig_t& intbig_t::to_power(const intbig_t& pow)
{
    return operator=(at_power(pow));
}

intbig_t intbig_t::at_power(const intbig_t& pow) const
{
    if(pow.sign < 0) {
        throw std::logic_error("Can't raise to negative power " + pow.to_string());
    }
    else if(pow.sign == 0) {
        return of(1);
    }

    intbig_t result = of(1);

    intbig_t pow2_this = *this;

    for(size_t i = 0; i < pow.num_bits(); i++) {
        if(pow.test_bit(i)) {
            result *= pow2_this;
        }

        pow2_this.square();
    }

    return result;
}

intbig_t& intbig_t::mul_mod(const intbig_t& other, const intbig_t& m)
{
    return operator=(times_mod(other, m));
}

intbig_t intbig_t::times_mod(const intbig_t& other, const intbig_t& m) const
{
    if(sign < 0 || other.sign < 0 || m.sign <= 0) {
        throw std::logic_error("");
    }
    else if(!sign || !other.sign) {
        // TODO: Do the modulo on operands before this check

        return of(0);
    }

    intbig_t result;

    intbig_t pow2_this = operator%(m);
    const intbig_t m_other = other % m;

    for(size_t i = 0; i < other.num_bits(); i++) {
        if(other.test_bit(i)) {
            result += pow2_this;

            if(result >= m) {
                result -= m;
            }
        }

        pow2_this += pow2_this;

        if(pow2_this >= m) {
            pow2_this -= m;
        }
    }

    return result;
}

intbig_t& intbig_t::to_power(const intbig_t& pow, const intbig_t& m)
{
    return operator=(at_power(pow, m));
}

intbig_t intbig_t::at_power(const intbig_t& pow, const intbig_t& m) const
{
    if(sign < 0 || pow.sign < 0 || m.sign <= 0) {
        throw std::logic_error("");
    }
    else if(!pow.sign) {
        return of(1);
    }

    intbig_t result = of(1);

    intbig_t pow2_this = operator%(m);

    for(size_t i = 0; i < pow.num_bits(); i++) {
        if(pow.test_bit(i)) {
            result *= pow2_this;

            if(result >= m) {
                result %= m;
            }
        }

        pow2_this.square();

        if(pow2_this >= m) {
            pow2_this %= m;
        }
    }

    return result;
}

void euclid_ex(const intbig_t& a, const intbig_t& b, intbig_t& x, intbig_t& y)
{
    if(a == 0) {
        x = intbig_t::of(0);
        y = intbig_t::of(1);
        return;
    }

    intbig_t x_new, y_new;
    euclid_ex(b % a, a, x_new, y_new);

    x = y_new - (b / a) * x_new;
    y = x_new;
}

intbig_t intbig_t::inverse_mod(const intbig_t& m) const
{
    intbig_t x, y;
    euclid_ex(*this, m, x, y);

    if(x < 0) {
        // REVIEW: Is this enough to get it out of the negatives?
        x += m;
    }
    else if(x >= m) {
        x %= m;
    }

    return x;
}

namespace
{
    int64_t gcd1(int64_t a, int64_t b)
    {
        while(b != 0) {
            a %= b;
            std::swap(a, b);
        }

        return a;
    }
}

int64_t intbig_t::gcd(const int64_t x) const
{
    return gcd1(x, operator%(x));
}

intbig_t intbig_t::gcd(const intbig_t& other) const
{
    if(!sign) {
        return other;
    }
    else if(!other.sign) {
        return *this;
    }

    /**
     * https://en.wikipedia.org/wiki/Binary_GCD_algorithm
     */

    intbig_t u = *this, v = other;
    uint64_t coef2 = 0;

    while(u != 0 && v != 0) {
        const bool u_even = u % 2 == 0;
        const bool v_even = v % 2 == 0;

        if(u_even && v_even) {
            coef2 += 1;

            u >>= 1;
            v >>= 1;
        }
        else if(u_even) {
            u >>= 1;
        }
        else if(v_even) {
            v >>= 1;
        }
        else {
            if(u < v) {
                v -= u;
                v >>= 1;
            }
            else {
                u -= v;
                u >>= 1;
            }
        }
    }

    if(u != 0) {
        return u << coef2;
    }
    else {
        return v << coef2;
    }
}
