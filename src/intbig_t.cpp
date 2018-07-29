#include <stdexcept>

#include "intbig_t.h"

// REMOVE: temporary, for the following:
// REMOVE:  - to_string
// REMOVE:  - from_decimal
#include "InfInt.h"

// TODO: decide how much to reserve
// TODO: decide whether to create some common constructor that just reserves
constexpr size_t INITIAL_RESEVATION = 20;

intbig_t::intbig_t(bool is_neg, std::vector<uint64_t>&& chunks) : is_neg(is_neg),
                                                                  chunks(std::move(chunks))
{
    chunks.reserve(INITIAL_RESEVATION);
}

intbig_t::intbig_t(int64_t x) : is_neg(x < 0)
{
    chunks.reserve(INITIAL_RESEVATION);

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

int intbig_t::compare_3way(const intbig_t& other) const
{
    // Since C++20, there's <=>

    if(is_neg != other.is_neg) {
        // This is legal: (int) of bool is 1 or 0
        // https://en.cppreference.com/w/cpp/language/implicit_conversion#Integral_promotion
        return other.is_neg - is_neg;
    }

    {
        // No leading zeroes are allowed, so longed value is neccessarily larger
        int size_diff = (int)chunks.size() - (int)other.chunks.size();

        if(size_diff != 0) {
            return (is_neg ? -1 : 1) * size_diff;
        }
    }

    // From most significant to least significant
    for(size_t i_back = 0; i_back < chunks.size(); i_back++) {
        /*
         * Can't simply return a - b here:
         *   a) it may not fit neither int nor int64_t;
         *   b) it will be unsigned, so some additional work is required anyway.
         */

        const uint64_t our_chunk = chunks[(chunks.size() - 1) - i_back];
        const uint64_t their_chunk = other.chunks[(chunks.size() - 1) - i_back];

        if(our_chunk < their_chunk) {
            return is_neg ? 1 : -1;
        }
        else if(our_chunk > their_chunk) {
            return is_neg ? -1 : 1;
        }
    }

    return 0;
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
     * So the only difference between the twoo is explicit copy versus implicit. Phew. Better stick with explicit one.
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

void intbig_t::operator+=(const intbig_t& other)
{
    if(is_neg || other.is_neg) {
        // TODO: implement through unary and binary `-`s
        throw std::logic_error("Only positives, please");
    }

    if(other.chunks.size() > chunks.size()) {
        chunks.resize(other.chunks.size());
    }

    bool carry = false;

    // other.chunks.size() <= chunks.size(), so we never iterate through too many
    for(size_t i = 0; i < other.chunks.size(); i++) {
        chunks[i] += other.chunks[i] + carry;

        // Set carry if overflow occured
        if(!carry) {
            carry = chunks[i] < other.chunks[i];
        }
        else {
            carry = chunks[i] <= other.chunks[i];
        }
    }

    // TODO: carry the leftover carry
}
