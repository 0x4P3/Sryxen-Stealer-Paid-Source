//
// Created by taxis on 2023-12-05.
//

#ifndef BROWSER_PATHS_HPP
#define BROWSER_PATHS_HPP

#include <string>
#include <vector>
#include <filesystem>

enum class BrowserType {
    CHROMIUM,
    GECKO
};

struct BrowserDir {
    BrowserType type;
    std::string profile_path;
    std::string local_state;
    std::string passwords;
    std::string cookies;
    std::string autofill;
    std::string history;
    std::string bookmarks;
};

namespace BPath {
    std::vector<BrowserDir> GetBrowserPaths();
}

#endif // BROWSER_PATHS_HPP
