
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
     * Product of small primes for use in divisibility testing through GCD.
     *
     * The 131 consecutive primes from 3 to 743  are used, as in SP800-89 5.3.3.
     */
    const intbig_t P_SMALL_PRIMES = intbig_t::from("1451887755777639901511587432083070202422614380984889313550570919659315177065956574359078912654149167643992684236991305777574330831666511589145701059710742276692757882915756220901998212975756543223550490431013061082131040808010565293748926901442915057819663730454818359472391642885328171302299245556663073719855");

    static int num_mr_checks(size_t n_bits)
    {
        /**
         * Number of Miller-Rabin iterations for error rate less than 2^-80
         *
         * See Handbook of Applied Cryptography, ch. 4, table 4.4
         */

        if(n_bits < 150) return 27;
        if(n_bits < 200) return 18;
        if(n_bits < 250) return 15;
        if(n_bits < 300) return 12;
        if(n_bits < 350) return 9;
        if(n_bits < 400) return 8;
        if(n_bits < 450) return 7;
        if(n_bits < 550) return 6;
        if(n_bits < 650) return 5;
        if(n_bits < 850) return 4;
        if(n_bits < 1300) return 3;

        return 2;
    }

    bool test_prime_mr(const intbig_t& n);

    intbig_t random_prime(size_t n_bits);
};

}

#endif //RSA_PREP_PRIMES_HPP
