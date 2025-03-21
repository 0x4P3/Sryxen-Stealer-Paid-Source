#ifndef COOKIES_HPP
#define COOKIES_HPP

#include <string>
#include <vector>

struct Cookie {
    std::string site;
    std::string name;
    std::string value;
    std::string path;
    std::string expires;
    bool is_secure;

};

namespace Browser {
    std::vector<Cookie> GetCookies();
}

#endif // COOKIES_HPP
