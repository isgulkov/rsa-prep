
#include <iostream>

enum oper_mode_t
{
    GEN_KEY, ENCRYPT, DECRYPT, CLEARSIGN, VERIFY
};

void print_usage()
{
    std::cerr << "Usage:" << '\n'
              << "  \33[4migpg\33[0m \33[4mgen-key\33[0m <n-bits>" << '\n'
              << "  \33[4migpg\33[0m \33[4mencrypt\33[0m ..." << '\n'
              << "  \33[4migpg\33[0m \33[4mdecrypt\33[0m ..." << '\n'
              << "  \33[4migpg\33[0m \33[4mclearsign\33[0m ..." << '\n'
              << "  \33[4migpg\33[0m \33[4mverify\33[0m ..." << std::endl;
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

int main(int argc, char** argv)
{
    if(argc == 1) {
        print_usage();
        return 1;
    }

    const std::string cmd = argv[1];

    if(cmd == "gen-key") {

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
