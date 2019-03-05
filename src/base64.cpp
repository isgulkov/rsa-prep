
#include "base64.hpp"

#include <array>

char encode(uint8_t v)
{
    // TODO: accept uint here!

    if(v < 26) {
        return (char)('A' + v);
    }
    else if(v < 52) {
        return (char)('a' - 26 + v);
    }
    else if(v < 62) {
        return (char)('0' - 52 + v);
    }
    else if(v == 62) {
        return '+';
    }
    else if(v == 63) {
        return '/';
    }

    throw std::invalid_argument("Can't encode value `" + std::to_string(v) + "`");
}

uint8_t decode(char c)
{
    if(c >= 'A' && c <= 'Z') {
        return uint8_t(c - 'A');
    }
    else if(c >= 'a' && c <= 'z') {
        return uint8_t(c - 'a' + 26);
    }
    else if(c >= '0' && c <= '9') {
        return uint8_t(c - '0' + 52);
    }
    else if(c == '+') {
        return 62;
    }
    else if(c == '/') {
        return 63;
    }
    else if(c == '=') {
        return 0;
    }

    throw std::invalid_argument("Can't decode character `" + std::to_string(c) + "`");
}

bool is_whitespace(char c)
{
    switch(c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            return true;
        default:
            return false;
    }
}

/**
 * +--first octet--+-second octet--+--third octet--+
 * |7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|7 6 5 4 3 2 1 0|
 * +-----------+---+-------+-------+---+-----------+
 * |5 4 3 2 1 0|5 4 3 2 1 0|5 4 3 2 1 0|5 4 3 2 1 0|
 * +--1.index--+--2.index--+--3.index--+--4.index--+
 *
 * TODO:
 *  - decode: fail (with an exception) on unexpected characters
 *  - encode: insert newlines every so many characters?
 */
namespace base64
{

std::string b64encode(const std::string& s)
{
    std::string result;

    for(size_t i = 0; i < s.size(); ) {
        const auto b0 = (uint8_t)s[i++],
            b1 = uint8_t(i < s.size() ? s[i++] : 0),
            b2 = uint8_t(i < s.size() ? s[i++] : 0);

        result += encode(b0 >> 2);
        result += encode(((b0 & uint8_t(0x3)) << 4) | (b1 >> 4));
        result += encode(((b1 & uint8_t(0xF)) << 2) | (b2 >> 6));
        result += encode(b2 & uint8_t(0x3F));
    }

    switch(s.size() % 3) {
        case 1:
            result[result.size() - 2] = '=';
        case 2:
            result.back() = '=';
        default:
            return result;
    }
}

std::string b64decode(const std::string& s)
{
    std::string result;

    std::array<uint8_t, 4> vs { 0, 0, 0, 0 };

    for(size_t i = 0; i < s.size(); ) {
        for(size_t i_v = 0; i_v < 4; i_v++) {
            while(i < s.size() && is_whitespace(s[i])) {
                i++;
            }

            if(i == s.size()) {
                if(i_v) {
                    throw std::invalid_argument(
                            "Unexpected end of string (" + std::to_string(3 - i_v) + "chars missing)"
                    );
                }

                goto end;
            }

            vs[i_v] = decode(s[i++]);
        }

        result += char((vs[0] << 2) | (vs[1] >> 4));
        result += char(((vs[1] & 0xF) << 4) | (vs[2] >> 2));
        result += char(((vs[2] & 0x3) << 6) | vs[3]);
    }

    end:

    // TODO: fix the handling of '=' characters here and in decode()
    for(ssize_t i = s.size() - 1; i >= 0; i--) {
        if(s[i] == '=') {
            result.pop_back();
        }
        else if(!is_whitespace(s[i])) {
            break;
        }
    }

    return result;
}

}
