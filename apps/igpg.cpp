
#include <iostream>
#include <fstream>

#include "base64.hpp"
#include "rsa.hpp"

using namespace isg;

void print_usage()
{
    std::cerr << "Usage:" << '\n'
              << "  \33[4migpg\33[0m \33[4mgen-key\33[0m <n-bits>" << '\n'
              << "  \33[4migpg\33[0m \33[4mencrypt\33[0m -k <pub-key>" << '\n'
              << "  \33[4migpg\33[0m \33[4mdecrypt\33[0m -k <priv-key>" << '\n'
              << "  \33[4migpg\33[0m \33[4mclearsign\33[0m -k <priv-key>" << '\n'
              << "  \33[4migpg\33[0m \33[4mverify\33[0m -k <pub-key>" << std::endl;
}

void print_version()
{
    std::cout << "igpg (isgPG) 0.0.1" << '\n'
              << "Copyright (C) 2019 Ilya Gulkov" << '\n'
              << '\n'
//              << "Home: /Users/username/.igpg" << '\n'
              << "Supported algorithms:" << '\n'
              << "Pubkey: RSA" << '\n'
              << "Cipher: AES256" << '\n'
              << "Hash: SHA256" << '\n'
              << "Compression: Uncompressed"
//              << "Compression: Uncompressed, ZIP"
              << std::endl;
}

void dump_pubkey(const rsa::key_pub& pub, std::ostream& os)
{
    os << "-----BEGIN RSA PUBLIC KEY-----" << '\n'
              << "Version: isgPG v0.0.1" << '\n' << '\n';

    os << base64::b64encode(pub.to_packet(), 64) << '\n' << '\n';

    os << "-----END RSA PUBLIC KEY-----" << std::endl;
}

void dump_privkey(const rsa::key_priv& priv, std::ostream& os)
{
    os << "-----BEGIN RSA PRIVATE KEY-----" << '\n'
       << "Version: isgPG v0.0.1" << '\n' << '\n';

    os << base64::b64encode(priv.to_packet(), 64) << '\n' << '\n';

    os << "-----END RSA PRIVATE KEY-----" << std::endl;
}

int main(int argc, char** argv)
{
    if(argc == 1) {
        print_usage();
        return 1;
    }

    const std::string cmd = argv[1];

    if(cmd == "gen-key") {
        const size_t n_bits = std::stoull(argv[2]);

        std::cerr << "Will generate a \33[1m" << n_bits << "\33[0m-bit RSA key pair..." << std::endl;

        const auto keypair = rsa::gen_keypair(n_bits);

        const rsa::key_pub pub = keypair.first;
        const rsa::key_priv priv = keypair.second;

        std::cerr << "Writing \33[1m" << "1337.pub" << "\33[0m..." << std::endl;
        std::ofstream f_out_pub("1337.pub");
        dump_pubkey(pub, f_out_pub);

        std::cerr << "Writing \33[1m" << "1337.priv" << "\33[0m..." << std::endl;
        std::ofstream f_out_priv("1337.priv");
        dump_privkey(priv, f_out_priv);
    }
    else if(cmd == "encrypt") {

    }
    else if(cmd == "decrypt") {

    }
    else if(cmd == "clearsign") {

    }
    else if(cmd == "verify") {

    }
    else if(cmd == "--version" || cmd == "-v") {
        print_version();
    }
    else {
        print_usage();
        return 1;
    }

    return 0;
}
