
#include "base64.hpp"

std::string read_cin()
{
    std::string s;
    char c;

    while(std::cin.get(c)) {
        s += c;
    }

    return s;
}

void print_usage()
{
    std::cerr << "Usage:" << '\n';
    std::cerr << "  b64codec --encode" << '\n';
    std::cerr << "  b64codec --decode" << '\n';
}

/**
 * A command-line base64 codec utility.
 *
 * TODO: input file argument (after streams are implemented)
 * TODO: print usage on -h argument
 */
int main(int argc, char** argv)
{
    if(argc != 2) {
        print_usage();
        return 1;
    }

    if(argv[1] == std::string("--encode")) {
        std::cout << base64::b64encode(read_cin()) << std::flush;
    }
    else if(argv[1] == std::string("--decode")) {
        std::cout << base64::b64decode(read_cin()) << std::flush;
    }
    else {
        print_usage();
        return 1;
    }

    return 0;
}
