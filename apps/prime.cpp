
#include <iostream>

#include "primes.hpp"

int main(int argc, char** argv)
{
    if(argc == 1) {
        std::cerr << "Usage:\n"
                  << "  \33[4mprime\33[0m [<n-bits>...]" << std::endl;
        return 1;
    }

    using namespace isg;

    prime_finder pf(true);

    for(int i = 1; i < argc; i++) {
        const uint64_t n_bits = std::stoull(argv[i]);

        std::cout << pf.random_prime(n_bits) << std::endl;
    }

    return 0;
}
