#include <iostream>
#include <functional>

#include "intbig_t.h"

// REMOVE: temporary dependency of intbig_t -- while it doesn't have its own multiply and divide
extern "C" {
#include "mini-gmp.h"
#include "mini-gmp.c"
}

class calc {
    std::vector<intbig_t> stack;

public:
    void push(const intbig_t& x)
    {
        stack.push_back(x);
    }

private:
    void apply_bin(const std::function<intbig_t(const intbig_t&, const intbig_t&)>& op)
    {
        intbig_t b = stack.back();
        stack.pop_back();

        intbig_t a = stack.back();
        stack.pop_back();

        stack.push_back(op(a, b));
    }

    void apply_un(const std::function<intbig_t(const intbig_t&)>& op)
    {
        stack.back() = op(stack.back());
    }

public:
    void accept_word(const std::string& word)
    {
        std::cerr << '"' << word << '"' << std::endl;

        if(word == "+") {
            apply_bin([](const intbig_t& a, const intbig_t& b) { return a + b; });
        }
        else if(word == "negate") {
            apply_un([](const intbig_t& a) { return -a; });
        }
        else {
            stack.push_back(intbig_t::from(word));
        }
    }

    intbig_t get_result()
    {
        if(stack.size() > 1) {
            std::string msg = "More than one element left (top to bottom):\n";

            while(!stack.empty()) {
                msg += stack.back().to_string() + '\n';
                stack.pop_back();
            }

            throw std::logic_error(msg);
        }

        return stack[0];
    }
};

/**
 * A simple calculator app for postfix expressions on multiple precision integers. Basically, an standard stream
 * interface to {@code intbig_t} for external testing.
 */
int main() {
    calc c;

    std::string expr;
    std::getline(std::cin, expr);

    size_t i = 0;

    while(true) {
        while(expr[i] == ' ' && i < expr.size()) {
            i += 1;
        }

        if(i == expr.size()) {
            break;
        }

        size_t j = i + 1;

        while(expr[j] != ' ' && j < expr.size()) {
            j += 1;
        }

        c.accept_word(expr.substr(i, j - i));

        i = j;
    }

    std::cout << c.get_result() << std::endl;

    return 0;
}
