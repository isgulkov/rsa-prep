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

intbig_t::intbig_t(bool is_neg, std::vector<uint64_t>&& chunks) : is_neg(is_neg),
                                                                  chunks(std::move(chunks))
{
    chunks.reserve(INITIAL_RESERVATION);
}

intbig_t::intbig_t(int64_t x) : is_neg(x < 0)
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

    bool is_neg = fifth_leg < 0;
    std::vector<uint64_t> chunks;

    if(fifth_leg < 0) {
        fifth_leg *= -1;
    }

    while(fifth_leg != 0) {
        chunks.push_back((fifth_leg % TWO_64).toUnsignedLongLong());

        fifth_leg /= TWO_64;
    }

    return { is_neg, std::move(chunks) };
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

    if(is_neg) {
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

    std::string result = is_neg ? "-" : " ";

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
    return is_neg == other.is_neg && chunks == other.chunks;
}

bool intbig_t::operator!=(const intbig_t& other) const
{
    return is_neg != other.is_neg || chunks != other.chunks;
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

    if(is_neg != other.is_neg) {
        // This is legal: bool gets casted to int values 1 and 0
        // https://en.cppreference.com/w/cpp/language/implicit_conversion#Integral_promotion
        return other.is_neg - is_neg;
    }

    return (is_neg ? -1 : 1) * compare_3way_unsigned(other);
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
            !is_neg && !chunks.empty(),  // Preserve false for zero
            std::vector<uint64_t>(chunks)  // Explicitly copy the vector for the && parameter
    );
}

intbig_t& intbig_t::negate()
{
    if(!chunks.empty()) {
        is_neg = !is_neg;
    }

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
         */

        // TODO: after sufficient testing, remove or put into #ifndef NDEBUG
        if(acc.size() < x.size()) {
            std::logic_error("it's less, you fuck!");
        }

        bool carry = false;

        // TODO: index of the "first leading zero": set to (i + 1) on non-zeroes
//        size_t i_first_lz = 0;

        for(size_t i = 0; i < x.size(); i++) {
            uint64_t acc_chunk = acc[i] - carry - x[i];

            // Set carry if overflow occured
            if(!carry) {
                carry = acc_chunk > acc[i];
            }
            else {
                carry = acc_chunk >= acc[i];
            }

            acc[i] = acc_chunk;
        }

        // Collect the remaining carry, if any
        for(size_t i = x.size(); carry && i < acc.size(); i++) {
            acc[i] -= carry;

            carry = (acc[i] == UINT64_MAX);
        }
        // TODO: how do you do ^this for sub2from?

        // TODO: after sufficient testing, remove or put into #ifndef NDEBUG
        // TODO: as well as "i < acc.size()" from loop's condition -- it serves no other purpose
        if(carry) {
            /*
             * This should never happen, so the check is just wasteful; but nobody wants an infinite loop in their
             * unit tests
             */
            std::logic_error("you lost your carry!");
        }

        while(acc.back() == 0) {
            acc.pop_back();
        }
        // REMOVE: after testing, replace this filth and implement the proper way:
//        if(acc.back() == 0) {
//            acc.resize(i_first_lz);
//        }
    }

    void sub2from_unsigned_(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x, bool in_reverse)
    {
        /*
         * In-place subtract the two unsigned big integers, but with reverse order of the operands:
         *   acc = x - acc
         *
         * Pre: x <= acc
         */

        // REMOVE: refactor these two, then either specialize this one or replace the other one with it

        //
        if(acc.size() != x.size() && acc.size() < x.size() == in_reverse) {
            std::logic_error("it's less, you fuck!");
        }

        bool carry = false;

        for(size_t i = 0; i < (!in_reverse ? x.size() : acc.size()); i++) {
            uint64_t chunk;

            if(!in_reverse) {
                chunk = acc[i] - x[i] - carry;
            }
            else {
                chunk = x[i] - acc[i] - carry;
            }

            if(!carry) {
                carry = chunk > acc[i];
            }
            else {
                carry = chunk >= acc[i];
            }

            acc[i] = chunk;
        }

        // TODO: extract the sizes as const size_t variables
        for(size_t i = (!in_reverse ? x.size() : acc.size()); i < (!in_reverse ? acc.size() : x.size()); i++) {
            if(in_reverse) {
                acc.push_back(x[i]);
            }

            if(carry) {
                carry = (acc[i] == UINT64_MAX);
            }
        }

        if(carry) {
            std::logic_error("you lost your carry! " + std::string(in_reverse ? "fuck" : "my life"));
        }

        while(acc.back() == 0) {
            acc.pop_back();
        }
        // REMOVE: after testing, replace this filth and implement the proper way:
//        if(acc.back() == 0) {
//            acc.resize(i_first_lz);
//        }
    }
    void sub2from_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x)
    {
        sub2from_unsigned_(acc, x, true);

        // REMOVE:
//        sub2from_unsigned_(acc, x, false);
    }
}

void intbig_t::add_abs(const intbig_t& other)
{
    is_neg = false;

    if(!other.chunks.empty()) {
        if(chunks.empty()) {
            chunks = other.chunks;
        }
        else {
            add2_unsigned(chunks, other.chunks);
        }
    }
}

void intbig_t::clear()
{
    chunks.resize(0);
    is_neg = false;
}

void intbig_t::sub_abs(const intbig_t& other)
{
    int cmp_with_other = compare_3way_unsigned(other);

    if(cmp_with_other < 0) {
        subfrom_abs(other);
        negate();
    }
    else if(cmp_with_other == 0) {
        // This is just an optimization -- the base case could handle this fine

        clear();
    }
    else {
        sub2_unsigned(chunks, other.chunks);
        is_neg = false;
    }
}

void intbig_t::subfrom_abs(const intbig_t& other)
{
    int cmp_with_other = compare_3way_unsigned(other);

    if(cmp_with_other < 0) {
        sub2from_unsigned(chunks, other.chunks);
        is_neg = false;
    }
    else if(cmp_with_other == 0) {
        clear();
    }
    else {
        sub_abs(other);
        negate();
    }
}

void intbig_t::operator+=(const intbig_t& other)
{
    if(!is_neg) {
        if(!other.is_neg) {
            // The base case:
            // a + b

            add_abs(other);
        }
        else {
            // a + -b --> a - b

            sub_abs(other);
        }
    }
    else {
        if(!other.is_neg) {
            // -a + b --> b - a

            subfrom_abs(other);
        }
        else {
            // -a + -b --> -(a + b)

            add_abs(other);
            negate();
        }
    }
}

void intbig_t::operator-=(const intbig_t& other)
{
    if(!is_neg) {
        if(!other.is_neg) {
            // The base case:
            // a - b

            sub_abs(other);
        }
        else {
            // a - -b --> a + b

            add_abs(other);
        }
    }
    else {
        if(!other.is_neg) {
            // -a - b --> -(a + b)

            add_abs(other);
            negate();
        }
        else {
            // -a - -b --> b - a

            subfrom_abs(other);
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
