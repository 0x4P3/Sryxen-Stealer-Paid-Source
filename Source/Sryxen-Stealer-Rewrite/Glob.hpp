#pragma once
#include <cassert>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

#ifdef GLOB_USE_GHC_FILESYSTEM
#include <ghc/filesystem.hpp>
#else
#include <filesystem>
#endif

#include "obfusheader.h"

namespace glob {

#ifdef GLOB_USE_GHC_FILESYSTEM
    namespace fs = ghc::filesystem;
#else
    namespace fs = std::filesystem;
#endif

    namespace {

        bool string_replace(std::string& str, const std::string& from, const std::string& to) {
            const std::size_t start_pos = str.find(from);
            if (start_pos == std::string::npos)
                return false;
            str.replace(start_pos, from.length(), to);
            return true;
        }

        std::string translate(const std::string& pattern) {
            std::size_t i = 0;
            const std::size_t n = pattern.size();
            std::string result_string;

            while (i < n) {
                const auto c = pattern[i];
                i += 1;
                if (c == '*') {
                    result_string += ".*";
                }
                else if (c == '?') {
                    result_string += ".";
                }
                else if (c == '[') {
                    auto j = i;
                    if (j < n && pattern[j] == '!') {
                        j += 1;
                    }
                    if (j < n && pattern[j] == ']') {
                        j += 1;
                    }
                    while (j < n && pattern[j] != ']') {
                        j += 1;
                    }
                    if (j >= n) {
                        result_string += "\\[";
                    }
                    else {
                        auto stuff = std::string(pattern.begin() + i, pattern.begin() + j);
                        if (stuff.find("--") == std::string::npos) {
                            string_replace(stuff, std::string{ "\\" }, std::string{ R"(\\)" });
                        }
                        else {
                            std::vector<std::string> chunks;
                            std::size_t k = 0;
                            if (pattern[i] == '!') {
                                k = i + 2;
                            }
                            else {
                                k = i + 1;
                            }

                            while (true) {
                                k = pattern.find("-", k, j);
                                if (k == std::string::npos) {
                                    break;
                                }
                                chunks.emplace_back(pattern.begin() + i, pattern.begin() + k);
                                i = k + 1;
                                k = k + 3;
                            }

                            chunks.emplace_back(pattern.begin() + i, pattern.begin() + j);
                            bool first = false;
                            for (auto& s : chunks) {
                                string_replace(s, std::string{ "\\" }, std::string{ R"(\\)" });
                                string_replace(s, std::string{ "-" }, std::string{ R"(\-)" });  // Fix for hyphen
                                if (first) {
                                    stuff += s;
                                    first = false;
                                }
                                else {
                                    stuff += "-" + s;
                                }
                            }
                        }

                        // Escape set operations
                        std::string result;
                        std::regex_replace(std::back_inserter(result),
                            stuff.begin(), stuff.end(),
                            std::regex(std::string{ R"([&~|])" }),
                            std::string{ R"(\\\1)" });
                        stuff = result;
                        i = j + 1;
                        if (stuff[0] == '!') {
                            stuff = "^" + std::string(stuff.begin() + 1, stuff.end());
                        }
                        else if (stuff[0] == '^' || stuff[0] == '[') {
                            stuff = "\\\\" + stuff;
                        }
                        result_string = result_string + "[" + stuff + "]";
                    }
                }
                else {
                    static std::string special_characters = "()[]{}?*+-|^$\\.&~# \t\n\r\v\f";
                    static std::map<int, std::string> special_characters_map;
                    if (special_characters_map.empty()) {
                        for (const auto& sc : special_characters) {
                            special_characters_map.insert(
                                std::make_pair(static_cast<int>(sc), std::string{ "\\" } + std::string(1, sc)));
                        }
                    }

                    if (special_characters.find(c) != std::string::npos) {
                        result_string += special_characters_map[static_cast<int>(c)];
                    }
                    else {
                        result_string += c;
                    }
                }
            }
            return std::string{ "((" } + result_string + std::string{ OBF(R"()|[\r\n])$)") };
        }

        std::regex compile_pattern(const std::string& pattern) {
            return std::regex(translate(pattern), std::regex::ECMAScript);
        }

        bool fnmatch(const fs::path& name, const std::string& pattern) {
            return std::regex_match(name.string(), compile_pattern(pattern));
        }

        std::vector<fs::path> filter(const std::vector<fs::path>& names, const std::string& pattern) {
            std::vector<fs::path> result;
            for (auto& name : names) {
                if (fnmatch(name, pattern)) {
                    result.push_back(name);
                }
            }
            return result;
        }

        fs::path expand_tilde(fs::path path) {
            if (path.empty()) return path;

#ifdef _WIN32
            const auto* home_variable = OBF("USERNAME");
#else
            const char* home_variable = OBF("USER");
#endif
            char* home;
            _dupenv_s(&home, nullptr, home_variable);
            if (home == nullptr) {
                throw std::invalid_argument(OBF("error: Unable to expand `~` - HOME environment variable not set."));
            }

            std::string s = path.string();  // mOIVNG the string init outside the `if` condition
            if (s[0] == '~') {
                s = std::string(home) + s.substr(1, s.size() - 1);  // had to fix the usage of s
                return fs::path{ s };
            }
            return path;
        }

        bool has_magic(const std::string& pathname) {
            static const auto magic_check = std::regex(OBF("([*?[])"));
            return std::regex_search(pathname, magic_check);
        }

        bool is_hidden(const std::string& pathname) {
            return std::regex_match(pathname, std::regex(OBF(R"(^(.*\/)*\.[^\.\/]+\/*$)")));
        }

        bool is_recursive(const std::string& pattern) { return pattern == "**"; }

        std::vector<fs::path> iter_directory(const fs::path& dirname, const bool dironly) {
            std::vector<fs::path> result;

            auto current_directory = dirname;
            if (current_directory.empty()) {
                current_directory = fs::current_path();
            }

            if (exists(current_directory)) {
                try {
                    for (auto& entry : fs::directory_iterator(
                        current_directory, fs::directory_options::follow_directory_symlink |
                        fs::directory_options::skip_permission_denied)) {
                        if (!dironly || entry.is_directory()) {
                            if (dirname.is_absolute()) {
                                result.push_back(entry.path());
                            }
                            else {
                                result.push_back(fs::relative(entry.path()));
                            }
                        }
                    }
                }
                catch (std::exception&) {
                    // not a directory
                    // do nothing
                }
            }

            return result;
        }

        // Recursively yields relative pathnames inside a literal directory.
        std::vector<fs::path> rlistdir(const fs::path& dirname, const bool dironly) {
            std::vector<fs::path> result;
            for (auto& x : iter_directory(dirname, dironly)) {
                if (!is_hidden(x.string())) {
                    result.push_back(x);
                    for (auto& y : rlistdir(x, dironly)) {
                        result.push_back(y);
                    }
                }
            }
            return result;
        }

        // This helper function recursively yields relative pathnames inside a literal
        // directory.
        std::vector<fs::path> glob2(const fs::path& dirname, [[maybe_unused]] const std::string& pattern,
            const bool dironly) {
            std::vector<fs::path> result;
            assert(is_recursive(pattern));
            for (auto& dir : rlistdir(dirname, dironly)) {
                result.push_back(dir);
            }
            return result;
        }

        // These 2 helper functions non-recursively glob inside a literal directory.
        // They return a list of basenames.  _glob1 accepts a pattern while _glob0
        // takes a literal basename (so it only has to check for its existence).
        std::vector<fs::path> glob1(const fs::path& dirname, const std::string& pattern,
            const bool dironly) {
            std::vector<fs::path> filtered_names;
            for (auto& n : iter_directory(dirname, dironly)) {
                if (!is_hidden(n.string())) {
                    filtered_names.push_back(n.filename());
                }
            }
            return filter(filtered_names, pattern);
        }

        std::vector<fs::path> glob0(const fs::path& dirname, const fs::path& basename,
            bool /*dironly*/) {
            std::vector<fs::path> result;
            if (basename.empty()) {
                if (fs::is_directory(dirname)) {
                    result = { basename };
                }
            }
            else {
                if (fs::exists(dirname / basename)) {
                    result = { basename };
                }
            }
            return result;
        }

        std::vector<fs::path> glob(const std::string& pathname, const bool recursive = false,
            const bool dironly = false) {
            std::vector<fs::path> result;

            auto path = fs::path(pathname);

            if (pathname[0] == '~') {
                path = expand_tilde(path);
            }

            auto dirname = path.parent_path();
            const auto basename = path.filename();

            if (!has_magic(pathname)) {
                assert(!dironly);
                if (!basename.empty()) {
                    if (fs::exists(path)) {
                        result.push_back(path);
                    }
                }
                else {
                    if (fs::is_directory(dirname)) {
                        result.push_back(path);
                    }
                }
                return result;
            }

            if (dirname.empty()) {
                if (recursive && is_recursive(basename.string())) {
                    return glob2(dirname, basename.string(), dironly);
                }
                else {
                    return glob1(dirname, basename.string(), dironly);
                }
            }

            std::vector<fs::path> dirs;
            if (dirname != fs::path(pathname) && has_magic(dirname.string())) {
                dirs = glob(dirname.string(), recursive, true);
            }
            else {
                dirs = { dirname };
            }

            std::function<std::vector<fs::path>(const fs::path&, const std::string&, bool)>
                glob_in_dir;
            if (has_magic(basename.string())) {
                if (recursive && is_recursive(basename.string())) {
                    glob_in_dir = glob2;
                }
                else {
                    glob_in_dir = glob1;
                }
            }
            else {
                glob_in_dir = glob0;
            }

            for (auto& d : dirs) {
                for (auto& name : glob_in_dir(d, basename.string(), dironly)) {
                    fs::path subresult = name;
                    if (name.parent_path().empty()) {
                        subresult = d / name;
                    }
                    result.push_back(subresult);
                }
            }

            return result;
        }

    } // namespace end

    static std::vector<fs::path> glob(const std::string& pathname) {
        return glob(pathname, false);
    }

    static std::vector<fs::path> rglob(const std::string& pathname) {
        return glob(pathname, true);
    }

    static std::vector<fs::path> glob(const std::vector<std::string>& pathnames) {
        std::vector<fs::path> result;
        for (auto& pathname : pathnames) {
            for (auto& match : glob(pathname, false)) {
                result.push_back(std::move(match));
            }
        }
        return result;
    }

    static std::vector<fs::path> rglob(const std::vector<std::string>& pathnames) {
        std::vector<fs::path> result;
        for (auto& pathname : pathnames) {
            for (auto& match : glob(pathname, true)) {
                result.push_back(std::move(match));
            }
        }
        return result;
    }

    static std::vector<fs::path> glob(const std::initializer_list<std::string>& pathnames) {
        return glob(std::vector<std::string>(pathnames));
    }

    static std::vector<fs::path> rglob(const std::initializer_list<std::string>& pathnames) {
        return rglob(std::vector<std::string>(pathnames));
    }

} // namespace glob
