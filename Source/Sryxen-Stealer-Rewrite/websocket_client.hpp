#pragma once
#include "network_utils.hpp"
#include "utilities.hpp"
#include <wincrypt.h>
#include <vector>

#pragma comment(lib, "crypt32.lib")

class WebSocketClient {
    SOCKET m_socket = INVALID_SOCKET;
    string m_host;
    int m_port;

public:
    WebSocketClient(const string& host, int port) : m_host(host), m_port(port) {}

    bool Connect() {
        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_socket == INVALID_SOCKET) return false;

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(m_port);
        InetPtonA(AF_INET, m_host.c_str(), &serverAddr.sin_addr);

        if (connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
            closesocket(m_socket);
            return false;
        }
        return true;
    }

    bool Handshake(const string& path) {
        string request = "GET " + path + " HTTP/1.1\r\n"
            "Host: " + m_host + ":" + to_string(m_port) + "\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            "Sec-WebSocket-Version: 13\r\n\r\n";

        if (send(m_socket, request.c_str(), request.size(), 0) == SOCKET_ERROR)
            return false;

        char buffer[1024];
        int bytesRead = recv(m_socket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) return false;

        return string(buffer, bytesRead).find("HTTP/1.1 101") != string::npos;
    }

    void Send(const string& message) {
        vector<unsigned char> frame{ 0x81 };
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
            uint64_t beLen = custom_ntohll(len);
            frame.insert(frame.end(), reinterpret_cast<char*>(&beLen), reinterpret_cast<char*>(&beLen) + 8);
        }

        unsigned char mask[4];
        HCRYPTPROV hProv;
        if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
            CryptGenRandom(hProv, 4, mask);
            CryptReleaseContext(hProv, 0);
        }
        else {
            random_device rd;
            uniform_int_distribution<int> dist(0, 255);
            for (int i = 0; i < 4; ++i) mask[i] = static_cast<unsigned char>(dist(rd));
        }

        frame.insert(frame.end(), mask, mask + 4);

        string masked = message;
        for (size_t i = 0; i < len; ++i)
            masked[i] ^= mask[i % 4];

        frame.insert(frame.end(), masked.begin(), masked.end());
        send(m_socket, reinterpret_cast<char*>(frame.data()), frame.size(), 0);
    }

    string Receive() {
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
            payloadLen = custom_ntohll(len);
        }

        unsigned char mask[4];
        if (masked && recv(m_socket, reinterpret_cast<char*>(mask), 4, 0) != 4)
            return "";

        string payload(payloadLen, '\0');
        size_t totalReceived = 0;
        while (totalReceived < payloadLen) {
            int count = recv(m_socket, &payload[totalReceived], payloadLen - totalReceived, 0);
            if (count <= 0) break;
            totalReceived += count;
        }

        if (masked) {
            for (size_t i = 0; i < payloadLen; ++i)
                payload[i] ^= mask[i % 4];
        }

        return payload;
    }

    ~WebSocketClient() { closesocket(m_socket); }
};