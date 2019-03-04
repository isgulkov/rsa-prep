#include <iostream>
#include <fstream>

#include "sha256.h"

void print_sum(std::istream& in, const char* filename)
{
    std::string data;
    char c;

    while(in.get(c)) {
        data += c;
    }

    std::cout << SHA256::sha256_hex(data) << "  " << filename << std::endl;
}

/**
 * A basic CLI hashing utility.
 *
 * When called without arguments, accepts input on stdin. Otherwise, each one is treated as a file path.
 *
 * TODO: print usage on -h argument
 * TODO: implement checking against a list (https://linux.die.net/man/1/shasum)
 */
int main(int argc, char** argv)
{
    if(argc == 1) {
        print_sum(std::cin, "-");
    }
    else {
        for(int i = 1; i < argc; i++) {
            std::ifstream in(argv[i]);

            if(in.fail()) {
                std::cerr << argv[i] << ": No such file (or something)" << std::endl;
                continue;
            }

            print_sum(in, argv[i]);
        }
    }

    return 0;
}
