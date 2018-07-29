#include <stdexcept>

#include "intbig_t.h"

// REMOVE: temporary, for the following:
// REMOVE:  - to_string
#include "InfInt.h"

intbig_t::intbig_t(bool is_neg, std::vector<uint64_t>&& chunks) : is_neg(is_neg),
                                                                  chunks(std::move(chunks))
{
    // TODO: decide how much to reserve
    // TODO: decide whether to reserve in the default (zero) constructor
    chunks.reserve(20);
}

intbig_t::intbig_t(int64_t x) : intbig_t(x < 0, { (uint64_t)(x < 0 ? -x : x) }) { }

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
