#ifndef ASN1_HPP
#define ASN1_HPP

#include <string>

struct Asn1Value {
    std::string iv;
    std::string data;
};

Asn1Value DecodeDER(const std::string& der);

#endif
