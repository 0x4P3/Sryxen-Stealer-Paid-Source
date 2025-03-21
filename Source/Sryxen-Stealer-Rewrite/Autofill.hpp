#ifndef AUTOFILL_HPP
#define AUTOFILL_HPP

#include <string>
#include <vector>

struct Autofill {
    std::string input;
    std::string value;

};

namespace Browser {
    std::vector<Autofill> GetAutofill();
}

#endif // AUTOFILL_HPP
