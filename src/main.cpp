#include <iostream>

#include "intbig_t.h"

int main()
{
    intbig_t x = 10LL;

    std::cout << x << std::endl;

    x += 1LL;

    std::cout << x.to_string() << std::endl;

    x = -10LL;

    std::cout << x << std::endl;

    return 0;
}
