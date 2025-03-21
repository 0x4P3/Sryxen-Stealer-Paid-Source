#ifndef STEAM_DUMPER_HPP
#define STEAM_DUMPER_HPP
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#include <regex>
#include <fstream>

HANDLE OpenProcessByName(const std::wstring& processName) {
    HANDLE hProcess = NULL;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return NULL;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (std::wstring(pe32.szExeFile) == processName) {
                hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return hProcess;
}

std::vector<std::string> ExtractStringsByRegex(const std::vector<char>& data, const std::regex& pattern) {
    std::vector<std::string> matchedStrings;
    std::string currentString;

    for (char ch : data) {
        if (std::isalnum(ch) || std::ispunct(ch) || std::isspace(ch)) {
            currentString += ch;
        }
        else {
            if (!currentString.empty() && std::regex_search(currentString, pattern)) {
                matchedStrings.push_back(currentString);
            }
            currentString.clear();
        }
    }

    if (!currentString.empty() && std::regex_search(currentString, pattern)) {
        matchedStrings.push_back(currentString);
    }

    return matchedStrings;
}

std::vector<std::string> ScanProcessMemory(HANDLE hProcess, const std::regex& pattern) {
    std::vector<std::string> results;
    MEMORY_BASIC_INFORMATION mbi;
    LPVOID address = 0;

    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) != 0) {
        if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE) == PAGE_READWRITE) {
            std::vector<char> buffer(mbi.RegionSize);
            SIZE_T bytesRead;

            if (ReadProcessMemory(hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead) && bytesRead > 0) {
                std::vector<std::string> extracted = ExtractStringsByRegex(buffer, pattern);
                results.insert(results.end(), extracted.begin(), extracted.end());
            }
        }

        address = reinterpret_cast<LPBYTE>(mbi.BaseAddress) + mbi.RegionSize;
    }

    return results;
}

int search() {
    std::wcout << L"Steam Token Dumper" << std::endl;

    HANDLE hProcess = OpenProcessByName(L"steam.exe");
    if (hProcess) {
        std::regex tokenPattern(R"(eyAidHlwIjogIkpXVCIsICJhbGciOiAiRWREU0EiIH0[0-9a-zA-Z\.\-_]+)");
        std::vector<std::string> tokens = ScanProcessMemory(hProcess, tokenPattern);

        if (!tokens.empty()) {
            for (const std::string& token : tokens) {
                std::cout << "Refresh token: " << token << "\n";
            }
        }
        else {
            std::cout << "No tokens found in process memory.\n";
        }

        CloseHandle(hProcess);
    }
    else {
        std::cout << "No steam process found!" << std::endl;
    }

    std::cout << "Scanning finished... (Press Enter to exit)";
    std::cin.get();
    return 0;
}
#endif