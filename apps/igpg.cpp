
#include <iostream>
#include <fstream>
#include <ctime>

#include "formats.hpp"
#include "rsa.hpp"

using namespace isg;

void print_usage()
{
    std::cerr << "Usage:" << '\n'
              << "  \33[4migpg\33[0m \33[4mgen-key\33[0m [-l <n-bits>] [-o <filename>]" << '\n'
              << "  \33[4migpg\33[0m \33[4mencrypt\33[0m -k <pub-key> [-i <input-file>]" << '\n'
              << "  \33[4migpg\33[0m \33[4mdecrypt\33[0m -k <priv-key> [-i <input-file>]" << '\n'
              << "  \33[4migpg\33[0m \33[4msign\33[0m -k <priv-key> [-i <input-file>]" << '\n'
              << "  \33[4migpg\33[0m \33[4mverify\33[0m -k <pub-key> -s <sig> [-i <input-file>]" << std::endl;
}

void print_version()
{
    std::cout << "igpg (isgPG) 0.1" << '\n'
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
        std::string f_name = std::to_string(std::time(nullptr));
        intbig_t e = intbig_t::of(65537);
        size_t n_bits = 1024;

        for(int i = 2; i < argc; i++) {
            const std::string arg = argv[i];

            if(arg == "-l") {
                n_bits = std::stoull(argv[++i]);
            }
            else if(arg == "-o") {
                f_name = argv[++i];
            }
            else if(arg == "-e") {
                e = intbig_t::from(argv[++i]);
            }
            else {
                std::cerr << "Error: unexpected argument '" << arg << "'" << std::endl;
                return 2;
            }
        }

        std::cerr << "Will generate a \33[1m" << n_bits << "\33[0m-bit RSA key pair (e=\33[1m" << e << "\33[0m)..." << std::endl;

        const auto keypair = rsa::gen_keypair(n_bits, e);

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
        std::string path_pubkey, path_fin;

        for(int i = 2; i < argc; i++) {
            const std::string arg = argv[i];

            if(arg == "-k") {
                path_pubkey = argv[++i];
            }
            else if(arg == "-i") {
                path_fin = argv[++i];
            }
            else {
                std::cerr << "Error: unexpected argument '" << arg << "'" << std::endl;
                return 2;
            }
        }

        if(path_pubkey.empty()) {
            std::cerr << "Error: path to public key unspecified" << std::endl;
            return 2;
        }

        std::cerr << "Reading public key \33[1m" << path_pubkey << "\33[0m..." << std::endl;

        std::ifstream f_in_pub(path_pubkey);
        const rsa::key_pub pub = formats::load_pubkey(f_in_pub);

        std::string msg;

        if(!path_fin.empty()) {
            std::cerr << "Reading \33[1m" << path_fin << "\33[0m..." << std::endl;

            std::ifstream f_in(path_fin);
            msg = read_all(f_in);
        }
        else {
            std::cerr << "Reading stdin..." << std::endl;

            msg = read_all(std::cin);
        }

        formats::dump_enc_message(std::cout, pub.encrypt_pkcs(msg));
    }
    else if(cmd == "decrypt") {
        std::string path_privkey, path_fin;

        for(int i = 2; i < argc; i++) {
            const std::string arg = argv[i];

            if(arg == "-k") {
                path_privkey = argv[++i];
            }
            else if(arg == "-i") {
                path_fin = argv[++i];
            }
            else {
                std::cerr << "Error: unexpected argument '" << path_privkey << "'" << std::endl;
                return 2;
            }
        }

        if(path_privkey.empty()) {
            std::cerr << "Error: path to private key unspecified" << std::endl;
            return 2;
        }

        std::cerr << "Reading private key \33[1m" << path_privkey << "\33[0m..." << std::endl;

        std::ifstream f_in_priv(path_privkey);
        const rsa::key_priv priv = formats::load_privkey(f_in_priv);

        std::string msg;

        if(!path_fin.empty()) {
            std::cerr << "Reading \33[1m" << path_fin << "\33[0m..." << std::endl;

            std::ifstream f_in(path_fin);
            msg = formats::load_enc_message(f_in);
        }
        else {
            std::cerr << "Reading stdin..." << std::endl;

            msg = formats::load_enc_message(std::cin);
        }

        std::cout << priv.decrypt_pkcs(msg);
    }
    else if(cmd == "sign") {
        std::string path_privkey, path_fin;

        for(int i = 2; i < argc; i++) {
            const std::string arg = argv[i];

            if(arg == "-k") {
                path_privkey = argv[++i];
            }
            else if(arg == "-i") {
                path_fin = argv[++i];
            }
            else {
                std::cerr << "Error: unexpected argument '" << arg << "'" << std::endl;
                return 2;
            }
        }

        if(path_privkey.empty()) {
            std::cerr << "Error: path to private key unspecified" << std::endl;
            return 2;
        }

        std::cerr << "Reading private key \33[1m" << path_privkey << "\33[0m..." << std::endl;

        std::ifstream f_in_priv(path_privkey);
        const rsa::key_priv priv = formats::load_privkey(f_in_priv);

        std::string msg;

        if(!path_fin.empty()) {
            std::cerr << "Reading \33[1m" << path_fin << "\33[0m..." << std::endl;

            std::ifstream f_in(path_fin);
            msg = read_all(f_in);
        }
        else {
            std::cerr << "Reading stdin..." << std::endl;

            msg = read_all(std::cin);
        }

        formats::dump_detached_sig(std::cout, priv.sign_pkcs(msg));
    }
    else if(cmd == "verify") {
        std::string path_pubkey, path_sig, path_fin;

        for(int i = 2; i < argc; i++) {
            const std::string arg = argv[i];

            if(arg == "-k") {
                path_pubkey = argv[++i];
            }
            else if(arg == "-s") {
                path_sig = argv[++i];
            }
            else if(arg == "-i") {
                path_fin = argv[++i];
            }
            else {
                std::cerr << "Error: unexpected argument '" << arg << "'" << std::endl;
                return 2;
            }
        }

        if(path_pubkey.empty()) {
            std::cerr << "Error: path to public key unspecified" << std::endl;
            return 2;
        }

        if(path_sig.empty()) {
            std::cerr << "Error: path to signature unspecified" << std::endl;
            return 2;
        }

        std::cerr << "Reading public key \33[1m" << path_pubkey << "\33[0m..." << std::endl;

        std::ifstream f_in_pub(path_pubkey);
        const rsa::key_pub pub = formats::load_pubkey(f_in_pub);

        std::cerr << "Reading signature \33[1m" << path_pubkey << "\33[0m..." << std::endl;

        std::ifstream f_in_sig(path_sig);
        const std::string sig = formats::load_detached_sig(f_in_sig);

        std::string msg;

        if(!path_fin.empty()) {
            std::cerr << "Reading \33[1m" << path_fin << "\33[0m..." << std::endl;

            std::ifstream f_in(path_fin);
            msg = read_all(f_in);
        }
        else {
            std::cerr << "Reading stdin..." << std::endl;

            msg = read_all(std::cin);
        }

        const bool result = pub.verify_pkcs(msg, sig);

        if(result) {
            std::cout << "\33[1;32mSignature matches\33[0m" << std::endl;
            return 0;
        }
        else {
            std::cout << "\33[1;31mSignature doesn't match\33[0m" << std::endl;
            return 1;
        }
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
