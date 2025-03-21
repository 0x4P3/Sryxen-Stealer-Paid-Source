#pragma once
#include <windows.h>
#include <string>
#include <tlhelp32.h>

using namespace std;

wstring string_to_wstring(const string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size);
    return wstr;
}

string wstring_to_string(const wstring& wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size, NULL, NULL);
    return str;
}

uint64_t custom_ntohll(uint64_t value) {
    return ((value & 0xFF) << 56) | ((value & 0xFF00) << 40) |
        ((value & 0xFF0000) << 24) | ((value & 0xFF000000) << 8) |
        ((value >> 8) & 0xFF000000) | ((value >> 24) & 0xFF0000) |
        ((value >> 40) & 0xFF00) | ((value >> 56) & 0xFF);
}

void TerminateChromeProcesses(const wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (!Process32FirstW(snapshot, &pe)) {
        CloseHandle(snapshot);
        return;
    }

    do {
        if (wcscmp(pe.szExeFile, processName.c_str()) == 0) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
            if (hProcess) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
    } while (Process32NextW(snapshot, &pe));

    CloseHandle(snapshot);
}