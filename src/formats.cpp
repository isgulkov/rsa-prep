
#include "formats.hpp"
#include "base64.hpp"

namespace isg {
namespace formats {

void dump_armor(std::ostream& os, const std::string& header, const std::string& content)
{
    os << "-----BEGIN " << header << "-----" << '\n';
//       << "Version: isgPG v0.0.1" << '\n' << '\n';

    os << base64::b64encode(content, 64) << '\n';

    os << "-----END " << header << "-----" << std::endl;
}

std::string load_armor(std::istream& is, const std::string& header)
{
    std::string line;

    bool found_start = false;

    while(std::getline(is, line)) {
        if(line.find("-----BEGIN " + header + "-----") != std::string::npos) {
            found_start = true;
            break;
        }
    }

    if(!found_start) {
        throw std::logic_error("...");
    }

    std::string content;

    while(std::getline(is, line)) {
        if(line.find("-----END " + header + "-----") != std::string::npos) {
            return base64::b64decode(content);
        }

        content += line;
    }

    throw std::logic_error("....");
}

rsa::key_pub load_pubkey(std::istream& is)
{
    return rsa::key_pub::from_packet(load_armor(is, "RSA PUBLIC KEY"));
}

void dump_pubkey(std::ostream& os, const rsa::key_pub& pub)
{
    dump_armor(os, "RSA PUBLIC KEY", pub.to_packet());
}

rsa::key_priv load_privkey(std::istream& is)
{
    return rsa::key_priv::from_packet(load_armor(is, "RSA PRIVATE KEY"));
}

void dump_privkey(std::ostream& os, const rsa::key_priv& priv)
{
    dump_armor(os, "RSA PRIVATE KEY", priv.to_packet());
}

std::string load_enc_message(std::istream& is)
{
    return load_armor(is, "RSA ENCRYPTED MESSAGE");
}

void dump_enc_message(std::ostream& os, const std::string& msg)
{
    dump_armor(os, "RSA ENCRYPTED MESSAGE", msg);
}

}
}
