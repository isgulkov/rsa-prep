#include <iostream>
#include <vector>

#include "intbig_t.h"

int main()
{
    intbig_t a = intbig_t::from_decimal("3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312415963462");
    intbig_t b = intbig_t::from_decimal("3161839487167763594107724496196279505125213113747786694642261808083621393592765616154012046788175944497933563312415963461");

    std::cout << a.to_hex() << std::endl;
    std::cout << b.to_hex() << std::endl;
    std::cout << (a - b).to_hex() << std::endl;
    std::cout << (a - b == 1) << std::endl;
    std::cout << std::endl;

    return 0;
}
