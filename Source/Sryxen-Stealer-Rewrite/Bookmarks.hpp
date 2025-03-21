#ifndef BOOKMARKS_HPP
#define BOOKMARKS_HPP

#include <vector>
#include <string>
#include "json.hpp"

struct Bookmark {
    std::string url;
    std::string name;
    std::string date_added;

};

namespace Browser {
    std::vector<Bookmark> ExtractChromiumBookmarks();
}

#endif // BOOKMARKS_HPP
