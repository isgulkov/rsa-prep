#include <stdexcept>

#include "intbig_t.h"

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

intbig_t::intbig_t(int sign, std::vector<uint64_t>&& chunks) : sign(sign), chunks(std::move(chunks))
{
    chunks.reserve(INITIAL_RESERVATION);
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
    chunks.reserve(INITIAL_RESERVATION);

    // Respect the representation of zero with empty vector
    if(x != 0) {
        chunks = { (uint64_t)(x < 0 ? -x : x) };
    }
}

intbig_t intbig_t::from_decimal(const std::string& decimal)
{
    // REMOVE: temporary, until proper string conversions are implemented

    InfInt fifth_leg(decimal);

    InfInt TWO_64 = InfInt(UINT64_MAX) + 1;

    int sign = fifth_leg < 0 ? -1 : (fifth_leg == 0 ? 0 : 1);
    std::vector<uint64_t> chunks;

    if(fifth_leg < 0) {
        fifth_leg *= -1;
    }

    while(fifth_leg != 0) {
        chunks.push_back((fifth_leg % TWO_64).toUnsignedLongLong());

        fifth_leg /= TWO_64;
    }

    return { sign, std::move(chunks) };
}

std::string intbig_t::to_string() const
{
    // REMOVE: temporary, until proper string conversions are implemented

    const uint64_t HALF_CHUNK = 1ULL << 32U;

    InfInt x;

    for(size_t i_back = 0; i_back < chunks.size(); i_back++) {
        x *= InfInt(HALF_CHUNK);
        x += chunks[(chunks.size() - 1) - i_back] >> 32U;

        x *= InfInt(HALF_CHUNK);
        x += chunks[(chunks.size() - 1) - i_back] & (HALF_CHUNK - 1);
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

std::string intbig_t::to_hex() const
{
    // REMOVE: dev version -- make more usable and possibly roundtrip-convertible

    if(chunks.empty()) {
        return "[  0]";
    }

    std::string result = sign < 0 ? "-" : " ";

    for(ssize_t i = chunks.size() - 1; i >= 0; i--) {
        result += " " + uint64_as_hex(chunks[i]);
    }

    return result;
}

std::ostream& operator<<(std::ostream& os, const intbig_t& value)
{
    return os << value.to_string();
}

bool intbig_t::operator==(const intbig_t& other) const
{
    return sign == other.sign && chunks == other.chunks;
}

bool intbig_t::operator!=(const intbig_t& other) const
{
    return sign != other.sign || chunks != other.chunks;
}

int intbig_t::compare_3way_unsigned(const intbig_t& other) const
{
    // No leading zeroes are allowed, so longer value is necessarily larger
    if(chunks.size() < other.chunks.size()) {
        return -1;
    }
    else if(chunks.size() > other.chunks.size()) {
        return 1;
    }

    // From most significant to least significant
    for(ssize_t i = chunks.size() - 1; i >= 0; i--) {
        /*
         * Can't simply return a - b here -- it may not fit neither int nor int64_t
         */

        // REVIEW: const uint64_t&
        // REVIEW: a way to make their signed difference fit some type?
        const uint64_t our_chunk = chunks[i];
        const uint64_t their_chunk = other.chunks[i];

        if(our_chunk < their_chunk) {
            return -1;
        }
        else if(our_chunk > their_chunk) {
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
            std::vector<uint64_t>(chunks)  // Explicitly copy the vector for the && parameter
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
            acc[i] += x[i] + carry;

            // Set carry if overflow occured
            if(!carry) {
                carry = acc[i] < x[i];
            }
            else {
                carry = acc[i] <= x[i];
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
            uint64_t acc_chunk = acc[i] - carry - x[i];

            // Set carry if overflow has occurred
            if(!carry) {
                carry = acc_chunk > acc[i];
            }
            else {
                carry = acc_chunk >= acc[i];
            }

            acc[i] = acc_chunk;
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
            uint64_t chunk = x[i] - acc[i] - carry;

            if(!carry) {
                carry = chunk > x[i];
            }
            else {
                carry = chunk >= x[i];
            }

            acc[i] = chunk;
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

    add2_unsigned(chunks, other.chunks);
    sign = 1;
}

void intbig_t::clear()
{
    chunks.resize(0);
    sign = 0;
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
        sub2from_unsigned(chunks, other.chunks);
        sign = -1;
    }
    else if(cmp_with_other == 0) {
        // This is just an optimization -- the base case could handle this fine
        clear();
    }
    else {
        sub2_unsigned(chunks, other.chunks);
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
        sub2from_unsigned(chunks, other.chunks);
        sign = 1;
    }
    else if(cmp_with_other == 0) {
        clear();
    }
    else {
        sub2_unsigned(chunks, other.chunks);
        sign = -1;
    }
}

void intbig_t::operator+=(const intbig_t& other)
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
}

void intbig_t::operator-=(const intbig_t& other)
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
