
#include "rsa.hpp"

#include <random>

#include "primes.hpp"
#include "sha256.h"

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

std::string calculate_hash(const std::string& msg, hash_sel_t hash)
{
    if(hash == SHA256) {
        return SHA256::sha256_bytes(msg);
    }
    else {
        throw std::logic_error("Unknown hash function `" + std::to_string(hash) + "`");
    }
}

key_pub::key_pub(const std::string& e_bytes, const std::string& n_bytes)
{
    e = intbig_t::from(e_bytes, intbig_t::Base256);
    n = intbig_t::from(n_bytes, intbig_t::Base256);
}

std::string key_pub::e_bytes() const
{
    return e.to_string(intbig_t::Base256);
}

std::string key_pub::n_bytes() const
{
    return n.to_string(intbig_t::Base256);
}

std::string key_pub::to_packet() const
{
    std::string packet = "\x01";

    {
        const std::string s_n = n_bytes();

        packet += char(s_n.size() / 256);
        packet += char(s_n.size() % 256);

        packet += s_n;
    }

    {
        const std::string s_e = e_bytes();

        packet += char(s_e.size() / 256);
        packet += char(s_e.size() % 256);

        packet += s_e;
    }

    return packet;
}

key_pub key_pub::from_packet(const std::string& packet)
{
    if(packet[0] != '\x01') {
        throw std::logic_error("");
    }

    const size_t l_n = uint8_t(packet[1]) * 256 + uint8_t(packet[2]);
    const std::string s_n = packet.substr(3, l_n);

    const size_t l_e = uint8_t(packet[3 + l_n]) * 256 + uint8_t(packet[3 + l_n + 1]);
    const std::string s_e = packet.substr(3 + l_n + 2, l_e);

    return { s_e, s_n };
}

std::string random_nz_pad(size_t l_bytes)
{
    std::random_device rd;
    std::uniform_int_distribution<char> dist;

    std::string s;

    while(l_bytes--) {
        char c;

        do {
            c = dist(rd);
        } while(!c);

        s += c;
    }

    return s;
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

std::string key_pub::encrypt_pkcs(const std::string& msg) const
{
    const size_t n_len = n.num_bits() / 8;

    if(msg.size() > n_len - 11) {
        throw std::range_error(
                "Message (" + std::to_string(msg.size()) + " bytes)" +
                " doesn't fit the modulus (" + std::to_string(n_len) + " full bytes)"
        );
    }

    const std::string enc_msg = std::string{ '\x00', '\x02' } + random_nz_pad(n_len - msg.size() - 3) + '\x00' + msg;

    return encrypt(enc_msg);
}

bool key_pub::verify_pkcs(const std::string& msg, const std::string& sig, hash_sel_t hash) const
{
    const std::string pad_digest = encrypt(sig);

    const size_t i_start = pad_digest.find('\x00', 1);

    if(i_start == std::string::npos) {
        return false;
    }

    const std::string digest = pad_digest.substr(i_start + 1);

    return digest == calculate_hash(msg, hash);
}

key_priv::key_priv(const std::string& d_bytes, const std::string& n_bytes)
{
    d = intbig_t::from(d_bytes, intbig_t::Base256);
    n = intbig_t::from(n_bytes, intbig_t::Base256);
}

std::string key_priv::d_bytes() const
{
    return d.to_string(intbig_t::Base256);
}

std::string key_priv::n_bytes() const
{
    return n.to_string(intbig_t::Base256);
}

std::string key_priv::to_packet() const
{
    std::string packet = "\x02";

    {
        const std::string s_n = n_bytes();

        packet += char(s_n.size() / 256);
        packet += char(s_n.size() % 256);

        packet += s_n;
    }

    {
        const std::string s_d = d_bytes();

        packet += char(s_d.size() / 256);
        packet += char(s_d.size() % 256);

        packet += s_d;
    }

    return packet;
}

key_priv key_priv::from_packet(const std::string& packet)
{
    if(packet[0] != '\x02') {
        throw std::logic_error("");
    }

    const size_t l_n = uint8_t(packet[1]) * 256 + uint8_t(packet[2]);
    const std::string s_n = packet.substr(3, l_n);

    const size_t l_d = uint8_t(packet[3 + l_n]) * 256 + uint8_t(packet[3 + l_n + 1]);
    const std::string s_d = packet.substr(3 + l_n + 2, l_d);

    return { s_d, s_n };
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

std::string key_priv::decrypt_pkcs(const std::string& msg) const
{
    const std::string enc_msg = decrypt(msg);

    const size_t i_start = enc_msg.find('\x00', 1);

    if(i_start == std::string::npos) {
        return "";
    }
    else {
        return enc_msg.substr(i_start + 1);
    }
}

std::string key_priv::sign_pkcs(const std::string& msg, hash_sel_t hash) const
{
    const std::string digest = calculate_hash(msg, hash);

    const size_t n_len = n.num_bits() / 8;

    const std::string pad_digest = std::string{ '\x00', '\x01' } + std::string(n_len - digest.size() - 3, char('\xff')) + '\x00' + digest;

    return decrypt(pad_digest);
}

}
}
