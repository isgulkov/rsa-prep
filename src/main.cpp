#include <iostream>
#include <vector>

#include "sha256.h"

int main()
{
    std::cout << SHA256::sha256_hex("") << std::endl;

    return 0;
}
