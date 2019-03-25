
#include "rsa.hpp"

#include "primes.hpp"

namespace isg {
namespace rsa {

std::pair<key_pub, key_priv> gen_keypair(const size_t l_mod, const intbig_t& e)
{
    prime_finder pf(true);

    const intbig_t p = pf.random_prime(l_mod / 2 - 3);
    const intbig_t q = pf.random_prime(l_mod / 2 + 3);

    const intbig_t n = p * q;

    const intbig_t lambda_n = (p - 1) * (q - 1) / (p - 1).gcd(q - 1);

    const intbig_t d = e.inverse_mod_p(lambda_n);

    return { key_pub(e, n), key_priv(d) };
}

}
}
