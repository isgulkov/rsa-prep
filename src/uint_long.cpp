#include "uint_long.h"

#include <stdexcept>
//#include <iostream>

/**
 * Construct a number from its decimal string representation.
 * @param s The number's decimal representation.
 * @throws std::invalid_argument If @p s is of invalid format.
 */
uint_long::uint_long(const std::string& s) { }

uint_long::uint_long(uint64_t x, bool neg) : is_negative(neg)
{
    if(x == 0) {
        return;
    }
    else {
        len = x > UINT32_MAX ? 2 : 1;

        data = new uint32_t[len];

        data[0] = (uint32_t)x;

        if(len == 2) {
            data[1] = (uint32_t)(x >> 32U);
        }
    }
}

uint_long::uint_long(int64_t x) : uint_long(x >= 0 ? (uint64_t)x : (uint64_t)(-x), x < 0) { }

bool uint_long::operator==(const uint_long& other) const
{
    if(len != other.len) {
        return false;
    }

    if(len == 0) {
        return true;
    }

    if(is_negative != other.is_negative) {
        return false;
    }

    for(size_t i = 0; i < len; i++) {
        if(data[i] != other.data[i]) {
            return false;
        }
    }

    return true;
}

bool uint_long::operator!=(const uint_long& other) const
{
    return !operator==(other);
}
