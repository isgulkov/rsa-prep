#include <iostream>
#include <iomanip>
#include <vector>

#include "intbig_t.h"

extern "C" {
#include "mini-gmp.h"
#include "mini-gmp.c"
}

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

std::string hui(const mpz_t& x)
{
    return (mpz_sgn(x) == -1 ? "- " : "  ") + uint64_as_hex(mpz_getlimbn(x, 1)) + " " + uint64_as_hex(mpz_getlimbn(x, 0));
}

int main()
{
    std::string s = "2109840582450070658310040166140836617";
    std::string t = "3959959789200537618582114387829736246";

    intbig_t x = intbig_t::from(s);
    intbig_t y = intbig_t::from(t);

    std::cout << x << std::endl;
    std::cout << y << std::endl;

    x *= y;

    std::cout << x << std::endl;
    std::cout << y << std::endl;

    return 0;
}
