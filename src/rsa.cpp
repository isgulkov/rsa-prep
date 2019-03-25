
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

    const intbig_t d = e.inverse_mod(lambda_n);

    return { key_pub(e, n), key_priv(d, n) };
}

key_pub::key_pub(const std::string& e_bytes, const std::string& n_bytes)
{
    e = intbig_t::from(e_bytes, intbig_t::Base256);
    n = intbig_t::from(n_bytes, intbig_t::Base256);
}

std::string key_pub::encrypt(const std::string& msg) const
{
    intbig_t x_msg = intbig_t::from(msg, intbig_t::Base256);

    if(x_msg >= n) {
        throw std::range_error("Message doesn't fit the modulus");
    }

    x_msg.to_power(e, n);

    return x_msg.to_string(intbig_t::Base256);
}

key_priv::key_priv(const std::string& d_bytes, const std::string& n_bytes)
{
    d = intbig_t::from(d_bytes, intbig_t::Base256);
    n = intbig_t::from(n_bytes, intbig_t::Base256);
}

std::string key_priv::decrypt(const std::string& msg) const
{
    intbig_t x_msg = intbig_t::from(msg, intbig_t::Base256);

    if(x_msg >= n) {
        throw std::range_error("Ciphertext doesn't fit the modulus");
    }

    x_msg.to_power(d, n);

    return x_msg.to_string(intbig_t::Base256);
}

}
}
