
#include "primes.hpp"

#include <iostream>

namespace isg
{

bool prime_finder::test_prime_mr(const intbig_t& n)
{
    const intbig_t n_dec = n - 1;

    const uint64_t coef2 = n_dec.factor2();

    // TODO: This has to be non-zero! Ensure either here or inside the method.
    const intbig_t a_random = intbig_t::random_lte(n_dec);
    intbig_t q = n_dec >> coef2;

    if(a_random.at_power(q, n) == 1) {
        return true;
    }

    for(uint64_t i = 0; i < coef2; i++) {
        if(a_random.at_power(q, n) == n_dec) {
            return true;
        }

        q <<= 1;
    }

    return false;
}

bool prime_finder::test_prime_lucas(const intbig_t& n)
{
    /**
     * TODO: implement
     */

    return true;
}

intbig_t prime_finder::random_prime(const size_t n_bits)
{
    // TODO: Parallelize into several threads?

    while(true) {
        intbig_t x = intbig_t::random_bits(n_bits);

        if(!x.test_bit(0)) {
            x += 1;
        }

        if(x.gcd(P_SMALL_PRIMES) != 1) {
            continue;
        }

        if(print_feedback) {
            std::cerr << '.' << std::flush;
        }

        {
            bool is_composite = false;

            // TODO: Determine an appropriate number of iterations
            for(int i = 0; i < num_mr_checks(n_bits); i++) {
                if(!test_prime_mr(x)) {
                    is_composite = true;
                    break;
                }

                if(print_feedback) {
                    std::cerr << '+' << std::flush;
                }
            }

            if(is_composite) {
                continue;
            }
        }

        if(!test_prime_lucas(x)) {
            continue;
        }

        if(print_feedback) {
            std::cerr << '*' << std::endl;
        }

        return x;
    }
}

}
