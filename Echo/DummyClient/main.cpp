#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>

#pragma comment(lib, "ws2_32.lib")

constexpr int PORT = 3030;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int BUFFER_SIZE = 1024;

void ErrorExit(const char* msg) {
    std::cerr << msg << " Error: " << WSAGetLastError() << std::endl;
    exit(EXIT_FAILURE);
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        ErrorExit("WSAStartup failed");
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        ErrorExit("Socket creation failed");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        ErrorExit("Connection to server failed");
    }

    std::cout << "Connected to IOCP server!" << std::endl;

    while (true)
    {
        char buffer[BUFFER_SIZE];
        std::cin >> buffer;
        
        if (*buffer == 'e')
        {
            break;
        }

        int bytesSent = send(clientSocket, buffer, strlen(buffer), 0);
        if (bytesSent == SOCKET_ERROR) {
            ErrorExit("Send failed");
        }
        std::cout << "Sent data: " << buffer << std::endl;

        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "Received from server: " << buffer << std::endl;
        }
        else if (bytesReceived == 0) {
            std::cout << "Server closed connection" << std::endl;
        }
        else {
            ErrorExit("Receive failed");
        }
    }

    closesocket(clientSocket);
    WSACleanup();

	return 0;
}