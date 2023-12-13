#include "Socket.h"
#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include "main.h"

bool bindSocket(SOCKET& imageSocket, const char* ip, const int port) {
    WSAData data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        std::cout << "WSAStartup fail!\n";
        return false;
    }
    else {
        std::cout << "WSAStartup ok!\n";
    }

    imageSocket = INVALID_SOCKET;
    imageSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (imageSocket == INVALID_SOCKET) {
        std::cout << "Error at socket()!\n";
        WSACleanup();
        return false;
    }
    else {
        std::cout << "socket() is ok!\n";
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    if (inet_pton(AF_INET, (const char*)ip, &service.sin_addr.s_addr) != 1) {
        std::cout << "invalid ip address!\n";
        closesocket(imageSocket);
        WSACleanup();
        return false;
    }
    service.sin_port = htons((u_short)port);

    if (bind(imageSocket, (sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind() faild!\n";
        closesocket(imageSocket);
        WSACleanup();
        return false;
    }
    else {
        std::cout << "bind() is ok!\n";
    }

    return true;
}

bool listenSocket(SOCKET& serverSocket, SOCKET& acceptSocket) {
    if (listen(serverSocket, 1) == SOCKET_ERROR) {
        std::cout << "error at listen()\n";
        WSACleanup();
        return false;
    }
    else {
        std::cout << "listen() ok!\n";
        std::cout << "listening...\n";
    }

    acceptSocket = accept(serverSocket, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET) {
        std::cout << "failed at accept()\n";
        WSACleanup();
        exit(-1);
    }
    else {
		std::cout << "connected!\n";
	}

    return true;
}

bool initServerSocket(SOCKET& serverSocket, SOCKET& acceptSocket, const char* ip, const int port) {
    return (bindSocket(serverSocket, ip, port) && listenSocket(serverSocket, acceptSocket));
}

void sendScreenResolution(SOCKET acceptSocket, int width, int height) {
    std::string widthStr = std::to_string(width);
    std::string heightStr = std::to_string(height);

	send(acceptSocket, widthStr.c_str(), 10, 0);
    send(acceptSocket, heightStr.c_str(), 10, 0);
}