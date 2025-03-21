#ifndef HISTORY_HPP
#define HISTORY_HPP

#include <string>
#include <vector>

struct History {
    std::string url;
    std::string title;
    int visit_count;
    std::string last_visit_time;
};

namespace Browser {
    std::vector<History> GetHistory(); 
}

#endif // HISTORY_HPP
