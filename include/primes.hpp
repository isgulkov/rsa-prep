
#ifndef RSA_PREP_PRIMES_HPP
#define RSA_PREP_PRIMES_HPP

#include "intbig_t.h"

namespace isg
{

class prime_finder
{
    bool print_feedback;

public:
    prime_finder(bool print_feedback = false) : print_feedback(print_feedback) { }

    /**
     * Product of the first 15 primes: 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47.
     *
     * This is used for divisibility testing through GCD.
     */
    const int64_t P_SMALL_PRIMES = 614889782588491410;

    bool test_prime_mr(const intbig_t& n);
    bool test_prime_lucas(const intbig_t& x);

    intbig_t random_prime(size_t n_bits);
};

}

#endif //RSA_PREP_PRIMES_HPP
