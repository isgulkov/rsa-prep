
#ifndef RSA_PREP_FORMATS_HPP
#define RSA_PREP_FORMATS_HPP

#include <iostream>

#include "rsa.hpp"

namespace isg {
namespace formats {

rsa::key_pub load_pubkey(std::istream& is);
void dump_pubkey(std::ostream& os, const rsa::key_pub& pub);

rsa::key_priv load_privkey(std::istream& is);
void dump_privkey(std::ostream& os, const rsa::key_priv& priv);

std::string load_enc_message(std::istream& is);
void dump_enc_message(std::ostream& os, const std::string& msg);

std::string load_detached_sig(std::istream& is);
void dump_detached_sig(std::ostream& os, const std::string& sig);

}
}

#endif //RSA_PREP_FORMATS_HPP
