#include <utility>


#ifndef RSA_PREP_RSA_HPP
#define RSA_PREP_RSA_HPP

#include <string>

#include "intbig_t.h"

namespace isg {
namespace rsa {

enum hash_sel_t { SHA256 };

class key_pub;
class key_priv;

std::pair<key_pub, key_priv> gen_keypair(size_t l_mod, const intbig_t& e = intbig_t::of(65537));

class key_pub
{
    intbig_t e, n;

    key_pub(intbig_t e, intbig_t n) : e(std::move(e)), n(std::move(n)) { }

public:
    key_pub(const std::string& e_bytes, const std::string& n_bytes);

    std::string e_bytes() const;
    std::string n_bytes() const;

    std::string to_packet() const;
    static key_pub from_packet(const std::string& packet);

    std::string encrypt(const std::string& msg) const;

    std::string encrypt_pkcs(const std::string& msg) const;
    bool verify_pkcs(const std::string& msg, const std::string& sig, hash_sel_t hash = SHA256) const;

    friend std::pair<key_pub, key_priv> gen_keypair(size_t, const intbig_t&);
};

class key_priv
{
    intbig_t d, n;

    key_priv(intbig_t d, intbig_t n) : d(std::move(d)), n(std::move(n)) { }

public:
    key_priv(const std::string& d_bytes, const std::string& n_bytes);

    std::string d_bytes() const;
    std::string n_bytes() const;

    std::string to_packet() const;
    static key_priv from_packet(const std::string& packet);

    std::string decrypt(const std::string& msg) const;

    std::string decrypt_pkcs(const std::string& msg) const;
    std::string sign_pkcs(const std::string& msg, hash_sel_t hash = SHA256) const;

    friend std::pair<key_pub, key_priv> gen_keypair(size_t, const intbig_t&);
};

}
}

#endif //RSA_PREP_RSA_HPP
