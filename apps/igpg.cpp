
#include <iostream>
#include <fstream>
#include <ctime>

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

void dump_armor(const std::string& header, const std::string& content, std::ostream& os)
{
    os << "-----BEGIN " << header << "-----" << '\n';
//       << "Version: isgPG v0.0.1" << '\n' << '\n';

    os << base64::b64encode(content, 64) << '\n';

    os << "-----END " << header << "-----" << std::endl;
}

std::string load_armor(const std::string& header, std::istream& is)
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

        const rsa::key_pub pub = keypair.first;
        const rsa::key_priv priv = keypair.second;

        const std::string f_name = std::to_string(std::time(nullptr));

        {
            std::cerr << "Writing \33[1m" << f_name << ".pub" << "\33[0m..." << std::endl;
            std::ofstream f_out_pub(f_name + ".pub");

            dump_armor("RSA PUBLIC KEY", pub.to_packet(), f_out_pub);
        }

        {
            std::cerr << "Writing \33[1m" << f_name << ".priv" << "\33[0m..." << std::endl;
            std::ofstream f_out_priv(f_name + ".priv");

            dump_armor("RSA PRIVATE KEY", priv.to_packet(), f_out_priv);
        }
    }
    else if(cmd == "encrypt") {
        const std::string path_pubkey = argv[2];
        std::ifstream f_in_pub(path_pubkey);

        std::cerr << "Reading public key \33[1m" << path_pubkey << "\33[0m..." << std::endl;

        const rsa::key_pub pub = rsa::key_pub::from_packet(load_armor("RSA PUBLIC KEY", f_in_pub));

        std::cerr << "Reading stdin..." << std::endl;

        const std::string msg = read_all(std::cin);

        dump_armor("RSA ENCRYPTED MESSAGE", pub.encrypt_pkcs(msg), std::cout);
    }
    else if(cmd == "decrypt") {
        const std::string path_privkey = argv[2];
        std::ifstream f_in_priv(path_privkey);

        std::cerr << "Reading private key \33[1m" << path_privkey << "\33[0m..." << std::endl;

        const rsa::key_priv priv = rsa::key_priv::from_packet(load_armor("RSA PRIVATE KEY", f_in_priv));

        std::cerr << "Reading stdin..." << std::endl;

        const std::string msg = priv.decrypt_pkcs(load_armor("RSA ENCRYPTED MESSAGE", std::cin));

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
