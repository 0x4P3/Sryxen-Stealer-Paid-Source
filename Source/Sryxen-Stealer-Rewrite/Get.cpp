#include "Get.hpp"

std::string FetchUrl(const wchar_t* host, const wchar_t* path, bool secure) {
    HINTERNET hSession = nullptr, hConnect = nullptr, hRequest = nullptr;
    std::string response;
    DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
    INTERNET_PORT port = secure ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;

    hSession = WinHttpOpen(OBF(L"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.5112.79 Safari/537.36"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) goto cleanup;

    hConnect = WinHttpConnect(hSession, host, port, 0);
    if (!hConnect) goto cleanup;

    hRequest = WinHttpOpenRequest(hConnect, OBF(L"GET"), path, nullptr, nullptr,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest || !WinHttpSendRequest(hRequest, nullptr, 0, nullptr, 0, 0, 0))
        goto cleanup;

    if (!WinHttpReceiveResponse(hRequest, nullptr)) goto cleanup;

    DWORD dwSize, dwDownloaded;
    char* pszOutBuffer;
    while (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize) {
        pszOutBuffer = new char[dwSize + 1];
        WinHttpReadData(hRequest, pszOutBuffer, dwSize, &dwDownloaded);
        response.append(pszOutBuffer, dwDownloaded);
        delete[] pszOutBuffer;
    }

cleanup:
    for (auto h : { hRequest, hConnect, hSession }) if (h) WinHttpCloseHandle(h);
    return response;
}
