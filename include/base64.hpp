
#ifndef RSA_PREP_BASE64_HPP
#define RSA_PREP_BASE64_HPP

#include <iostream>
#include <string>

namespace base64
{

std::string b64encode(const std::string&);
std::string b64decode(const std::string&);

// TODO: implement stream interface?
//void b64encode(std::istream& in, std::ostream& out);
//void b64decode(std::istream& in, std::ostream& out);

}

#endif //RSA_PREP_BASE64_HPP
