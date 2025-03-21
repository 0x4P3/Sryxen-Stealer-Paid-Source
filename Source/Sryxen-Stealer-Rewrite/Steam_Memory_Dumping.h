#ifndef STEAM_MEM_DUMP_H
#define STEAM_MEM_DUMP_H

#include <windows.h>
#include <vector>
#include <string>
#include <regex>
#include <iostream>

class ProcessScanner {
public:
    static void Start(); 

private:
    ProcessScanner(const std::wstring& processName, const std::regex& pattern);
    ~ProcessScanner();

    HANDLE hProcess;
    std::wstring processName;
    std::regex pattern;

    HANDLE OpenProcessByName();
    std::vector<std::string> ExtractStringsByRegex(const std::vector<char>& data);
    std::vector<std::string> ScanProcessMemory();
    void Run();
};

#endif // STEAM_MEM_DUMP_H
