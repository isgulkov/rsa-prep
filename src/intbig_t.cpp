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

void intbig_t::add2_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x)
{
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
        // TODO: can this be made with one logic+arithmetic expression instead of this if/else?
        // TODO: one option is add carry after the check, but then you'd have to store the new carry -- not pleasing
    }

    // Propagate the carry, if any, through the acc's higher digits
    for(size_t i = x.size(); i < acc.size(); i++) {
        acc[i] += carry;

        carry = (acc[i] == 0);
    }

    // potentially reaching a power of 64
    if(carry) {
        acc.push_back(1);
    }
}

void intbig_t::sub2_unsigned(std::vector<uint64_t>& acc, const std::vector<uint64_t>& x)
{
    // TODO
}

void intbig_t::operator+=(const intbig_t& other)
{
    // TODO: eliminate the need of copying

    // The idiot is doing this by just storing the [0; N) in, like, a [-2N; 2N] data type:
    //   - add digits elementwise (with the corresponding number's sign);
    //   - just somehow spread the carries through.
    // The 2x overhead does not seem to be worth it, though.

    // Handle non-positive operands separately
    if(is_neg || other.is_neg) {
        if(!is_neg) {
            // a += b --> a -= -b

            // Try to avoid copying b by reducing to sub2_unsigned
            // TODO: DRY

            // Make both negative to compare by absolute value
            // TODO: <-- abstract out from compare_3way its 'abs(x) <=> abs(y)' part for this spot here
            negate();
            int cmp_with_other = -compare_3way(other);
            negate();

            if(cmp_with_other < 0) {
                // a -= -b --> a = -b - a

                // BUG: 2 copies (other, result)
                *this = -other - *this;
            }
            else if(cmp_with_other == 0) {
                // a -= -b --> a = 0

                *this = 0;  // REVIEW: see analogous case in operator-=
            }
            else {
                // a -= -b

                sub2_unsigned(chunks, other.chunks);
            }
        }
        else if(!other.is_neg) {
            // a += b --> a = b - -a

            // BUG: 2 copies (this, result)
            *this = other - operator-();
        }
        else {
            // a += b --> a = -(-a + -b) --> |a| += |b|

            add2_unsigned(chunks, other.chunks);
        }
    }
    else {
        // Handle zero operands separately
        if(other.chunks.empty()) {
            // a += b --> no-op
        }
        else if(chunks.empty()) {
            // a += b --> a = b

            chunks = other.chunks;
        }
        else {
            // The base case: both operands are positive

            add2_unsigned(chunks, other.chunks);
        }
    }
}

void intbig_t::operator-=(const intbig_t& other)
{
    // Handle non-positive operands separately
    if(is_neg || other.is_neg) {
        // TODO: eliminate the need of copying

        if(!is_neg) {
            // a -= b --> a += -b

            add2_unsigned(chunks, other.chunks);
        }
        else if(!other.is_neg) {
            // a -= b --> a = -(a + b)

            negate();
            operator+=(other);
            negate();
        }
        else {
            // a -= b --> a = (b - a)

            // BUG: 2 copies (this, result)
            *this = other - operator-();
        }
    }

    int cmp_with_other = compare_3way(other);

    // Handle non-positive result separately
    if(cmp_with_other < -1) {
        // a -= b --> a = -(b - a)

        // BUG: 1 copy (result)
        *this = other - *this;
        negate();
    }
    else if(cmp_with_other == 0) {
        // a -= b --> a = 0
        // Base case should work on this, but it's unnecessary

        *this = 0;

        // REVIEW: check if modifying this object is any faster than move-assign:
        // REVIEW: we also currently lose the vector reserve buffer
//        chunks.resize(0);
//        is_neg = false;
    }
    else {
        // Handle zero operands separately
        if(other.chunks.empty()) {
            // a -= b --> no-op
        }
        else if(chunks.empty()) {
            // a -= b --> a = -b

            chunks = other.chunks;
            negate();  // So that zero stays a zero
        }
        else {
            // The base case:
            //   - both operands are positive;
            //   - a >= b, so the result is positive

            sub2_unsigned(chunks, other.chunks);
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
