
#include <iostream>
#include <fstream>
#include <ctime>

#include "formats.hpp"
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

std::string read_all(std::istream& is)
{
    std::string s;
    char c;

    while(is.get(c)) {
        s += c;
    }

    return s;
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

        const std::string f_name = std::to_string(std::time(nullptr));

        {
            std::cerr << "Writing \33[1m" << f_name << ".pub" << "\33[0m..." << std::endl;
            std::ofstream f_out_pub(f_name + ".pub");

            formats::dump_pubkey(f_out_pub, keypair.first);
        }

        {
            std::cerr << "Writing \33[1m" << f_name << ".priv" << "\33[0m..." << std::endl;
            std::ofstream f_out_priv(f_name + ".priv");

            formats::dump_privkey(f_out_priv, keypair.second);
        }
    }
    else if(cmd == "encrypt") {
        const std::string path_pubkey = argv[2];
        std::ifstream f_in_pub(path_pubkey);

        std::cerr << "Reading public key \33[1m" << path_pubkey << "\33[0m..." << std::endl;

        const rsa::key_pub pub = formats::load_pubkey(f_in_pub);

        std::cerr << "Reading stdin..." << std::endl;

        const std::string msg = read_all(std::cin);

        formats::dump_enc_message(std::cout, pub.encrypt_pkcs(msg));
    }
    else if(cmd == "decrypt") {
        const std::string path_privkey = argv[2];
        std::ifstream f_in_priv(path_privkey);

        std::cerr << "Reading private key \33[1m" << path_privkey << "\33[0m..." << std::endl;

        const rsa::key_priv priv = formats::load_privkey(f_in_priv);

        std::cerr << "Reading stdin..." << std::endl;

        const std::string msg = priv.decrypt_pkcs(formats::load_enc_message(std::cin));

        std::cout << msg << std::endl;
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
