#include <iostream>
#include <vector>

#include "intbig_t.h"
#include "InfInt.h"

int main()
{
    const std::string s = "6277101735386680763835789423207666416102355444464034512895";

    intbig_t a = intbig_t::from_decimal(s);
    InfInt b = s;

    std::cout << s << std::endl;
    std::cout << b.toString() << std::endl;
    std::cout << a.to_string() << std::endl;
    std::cout << a.to_hex() << std::endl;
    std::cout << std::endl;

    std::cout << "6277101735386680763835789423207666416102355444464034512896" << std::endl;
    std::cout << (b + 1).toString() << std::endl;
    std::cout << (a + 1).to_string() << std::endl;
    std::cout << (a + 1).to_hex() << std::endl;
    std::cout << std::endl;

    std::cout << "6277101735386680763835789423207666416102355444464034512894" << std::endl;
    std::cout << (b - 1).toString() << std::endl;
    std::cout << (a - 1).to_string() << std::endl;
    std::cout << (a - 1).to_hex() << std::endl;
    std::cout << std::endl;

    const std::string t = "35789423207666416102355444464034512894";

    a = intbig_t::from_decimal(t);
    b = t;

    std::cout << t << std::endl;
    std::cout << b.toString() << std::endl;
    std::cout << a.to_string() << std::endl;
    std::cout << a.to_hex() << std::endl;
    std::cout << std::endl;

    std::cout << "35789423207666416102355444464034512893" << std::endl;
    std::cout << (b - 1).toString() << std::endl;
    std::cout << (a - 1).to_string() << std::endl;
    std::cout << (a - 1).to_hex() << std::endl;
    std::cout << std::endl;

    return 0;
}
