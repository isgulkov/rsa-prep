#include <iostream>
#include <vector>

#include "intbig_t.h"
#include "InfInt.h"

InfInt power_of_2(uint64_t p)
{
    InfInt x = 1;

    while(p--) {
        x *= 2;
    }

    return x;
}

int main()
{
//    const std::string s = "879722148966781662298173139348";
//    const std::string t = "3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312415963462";

//    const std::string s = power_of_2(64 * 4).toString();
//    const std::string t = power_of_2(64 * 1).toString();

    const std::string s = "879722148966781662298173139348";
    const std::string t = "786977518832885148478497277741865824076420388401460";

//    const std::string s = (power_of_2(64 * 3) + power_of_2(64 * 2)).toString();
//    const std::string t = power_of_2(64 * 1).toString();

//    const std::string u = "92233720368547758080";

    intbig_t a = intbig_t::from_decimal(s);
    intbig_t b = intbig_t::from_decimal(t);
//    intbig_t c = intbig_t::from_decimal(u);

    std::cout << a.to_hex() << std::endl;
    std::cout << b.to_hex() << std::endl;
    std::cout << std::endl;

    std::cout << intbig_t::from_decimal((InfInt(s) - InfInt(t)).toString()).to_hex() << std::endl;
    std::cout << (a - b).to_hex() << std::endl;
    std::cout << (b - a).to_hex() << std::endl;
    std::cout << std::endl;

//    std::cout << (a - c).to_hex() << std::endl;
//    std::cout << (c - a).to_hex() << std::endl;
//    std::cout << std::endl;

    return 0;
}
