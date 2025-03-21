#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <random>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

using namespace std;

struct WSAInitializer {
    WSADATA wsaData;
    WSAInitializer() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) exit(1);
    }
    ~WSAInitializer() { WSACleanup(); }
};

inline WSAInitializer g_wsaInitializer;

bool IsPortAvailable(int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) return true;

    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        closesocket(sock);
        return true;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    InetPtonA(AF_INET, "127.0.0.1", &addr.sin_addr);

    int result = connect(sock, (sockaddr*)&addr, sizeof(addr));

    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            timeval timeout{ 1, 0 };
            fd_set writeSet;
            FD_ZERO(&writeSet);
            FD_SET(sock, &writeSet);

            result = select(0, NULL, &writeSet, NULL, &timeout);
            if (result > 0) {
                int error_code = 0;
                socklen_t len = sizeof(error_code);
                getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error_code, &len);
                if (error_code == 0) {
                    closesocket(sock);
                    return false;
                }
            }
        }
    }
    else {
        closesocket(sock);
        return false;
    }

    closesocket(sock);
    return true;
}

int GeneratePort() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(10000, 65535);

    int port;
    do { port = dist(gen); } while (!IsPortAvailable(port));
    return port;
}