#include "intbig_t.h"

#include <stdexcept>
//#include <iostream>
#include <cmath>

/**
 * Construct a number from its decimal string representation.
 * @param s The number's decimal representation.
 * @throws std::invalid_argument If @p s is of invalid format.
 */
intbig_t::intbig_t(const std::string& s) { }

intbig_t::intbig_t(uint64_t x, bool neg)
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

intbig_t::intbig_t(int64_t x) : intbig_t(x >= 0 ? (uint64_t)x : (uint64_t)(-x), x < 0) { }

bool intbig_t::operator==(const intbig_t& other) const
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

bool intbig_t::operator!=(const intbig_t& other) const
{
    return !operator==(other);
}

/**
 * @return - If the number is less than \p other, a negative number;
 *         - if they are equal, zero;
 *         - otherwise, a positive number.
 */
int intbig_t::compare(const intbig_t& other) const
{
    if(len < other.len) {
        return -1;
    }
    else if(len > other.len) {
        return 1;
    }

    // From most to least significant
    for(int i = std::abs(len) - 1; i >= 0; i--) {
        if(data[i] < other.data[i]) {
            return -len;
        }
        else if(data[i] > other.data[i]) {
            return len;
        }
    }

    return 0;
}

bool intbig_t::operator<(const intbig_t& other) const
{
    return compare(other) < 0;
}

bool intbig_t::operator<=(const intbig_t& other) const
{
    return compare(other) <= 0;
}

bool intbig_t::operator>=(const intbig_t& other) const
{
    return compare(other) >= 0;
}

bool intbig_t::operator>(const intbig_t& other) const
{
    return compare(other) > 0;
}

void intbig_t::resize_data(size_t new_size)
{
    // TODO: accept that boy signed ^
    // TODO: 0 -- only delete

    // Can't wait until the class is done and I'm getting my shiny std::vector instead of this

    if(new_size == std::abs(len)) {
        return;
    }

    auto* new_data = new uint32_t[new_size];

    memcpy(new_data, data, sizeof(uint32_t) * std::abs(len));
    memset(new_data + std::abs(len), 0, sizeof(uint32_t) * (new_size - std::abs(len)));

    delete[] data;
    data = new_data;

    if(len >= 0) {
        len = (int)new_size;
    }
    else {
        len = -(int)new_size;
    }
}

void intbig_t::operator+=(const intbig_t& other)
{
    if(std::abs(other.len) > std::abs(len)) {
        resize_data((size_t)std::abs(other.len));
    }

    uint32_t carry = 0;

    for(size_t i = 0; i < std::min(sz_data(), other.sz_data()); i++) {
        uint64_t sum = (uint64_t)data[i] + (uint64_t)other.data[i] + carry;

        data[i] = (uint32_t)(sum & 0xFFFFFFFF);

        carry = (uint32_t)(sum >> 32U);
    }

    // BUG: leftover carry shouldn't go discarded
}
