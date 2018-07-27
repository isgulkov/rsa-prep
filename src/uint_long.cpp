#include "uint_long.h"

#include <stdexcept>
//#include <iostream>
#include <cmath>

/**
 * Construct a number from its decimal string representation.
 * @param s The number's decimal representation.
 * @throws std::invalid_argument If @p s is of invalid format.
 */
uint_long::uint_long(const std::string& s) { }

uint_long::uint_long(uint64_t x, bool neg)
{
    if(x == 0) {
        return;
    }
    else {
        // TODO: rearrange somehow?

        len = x > UINT32_MAX ? 2 : 1;

        data = new uint32_t[len];

        data[0] = (uint32_t)x;

        if(len == 2) {
            data[1] = (uint32_t)(x >> 32U);
        }

        len = neg ? -len : len;
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

    for(size_t i = 0; i < std::abs(len); i++) {
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

bool uint_long::operator<(const uint_long& other) const
{
    if(len < other.len) {
        return true;
    }
    else if(len == other.len) {
        for(int i = std::abs(len) - 1; i >= 0; i--) {
            if(data[i] == other.data[i]) {
                continue;
            }

            if(len > 0) {
                return data[i] < other.data[i];
            }
            else {
                return data[i] > other.data[i];
            }
        }
    }

    return false;
}

bool uint_long::operator<=(const uint_long& other) const
{
    if(len < other.len) {
        return true;
    }
    else if(len == other.len) {
        for(int i = std::abs(len) - 1; i >= 0; i--) {
            if(data[i] == other.data[i]) {
                continue;
            }


            if(len > 0) {
                return data[i] < other.data[i];
            }
            else {
                return data[i] > other.data[i];
            }
        }

        return true;
    }

    return false;
}

bool uint_long::operator>=(const uint_long& other) const
{
    return !operator<(other);
}

bool uint_long::operator>(const uint_long& other) const
{
    return !operator<=(other);
}
