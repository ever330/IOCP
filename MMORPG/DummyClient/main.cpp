#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <thread>
#include <string>
#include "Packet.h"
#include "Define.h"

#pragma comment(lib, "ws2_32.lib")

constexpr int PORT = 3030;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int BUFFER_SIZE = 1024;

void ErrorExit(const char* msg) {
    std::cerr << msg << " Error: " << WSAGetLastError() << std::endl;
    exit(EXIT_FAILURE);
}

void RecvThread(SOCKET clientSocket)
{
    while (true)
    {
        char buffer[BUFFER_SIZE];
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

        if (bytesReceived > 0) 
        {
            // 받은 데이터가 최소한 패킷 헤더 크기 이상인지 확인
            if (bytesReceived < sizeof(PacketBase)) {
                std::cerr << "잘못된 패킷 크기: " << bytesReceived << std::endl;
                return;
            }

            // 패킷 헤더 추출
            PacketBase* pac = reinterpret_cast<PacketBase*>(buffer);

            // 받은 데이터 크기가 패킷에 명시된 크기보다 작은지 확인
            if (bytesReceived < pac->PacketSize) {
                std::cerr << "패킷 크기가 불완전함: " << bytesReceived << " / " << pac->PacketSize << std::endl;
                return;
            }

            int bodySize = pac->PacketSize - sizeof(PacketBase);

            // Body를 별도의 버퍼에 복사 (안전한 접근)
            char* bodyBuffer = new char[bodySize];
            memcpy(bodyBuffer, buffer + sizeof(PacketBase), bodySize);

            // 패킷 타입에 따라 처리
            if (pac->PacID == S2CSendMSG)
            {
                if (bodySize < sizeof(S2CSendMSGPacket)) {
                    std::cerr << "잘못된 메시지 패킷 크기!" << std::endl;
                    delete[] bodyBuffer;
                    return;
                }

                S2CSendMSGPacket msg;
                memcpy(&msg, bodyBuffer, sizeof(S2CSendMSGPacket));

                std::cout << msg.Name << " : " << msg.MSG << std::endl;
            }
            else if (pac->PacID == S2CNewUserAlert)
            {
                if (bodySize < sizeof(S2CNewUserAlertPacket)) {
                    std::cerr << "잘못된 메시지 패킷 크기!" << std::endl;
                    delete[] bodyBuffer;
                    return;
                }

                S2CNewUserAlertPacket alert;
                memcpy(&alert, bodyBuffer, sizeof(S2CSendMSGPacket));

                std::cout << alert.Name << " 님이 접속하셨습니다."  << std::endl;
            }

            delete[] bodyBuffer; // 동적 할당된 메모리 해제
        }
        else if (bytesReceived == 0) 
        {
            std::cout << "Server closed connection" << std::endl;
            break;
        }
        else 
        {
            ErrorExit("Receive failed");
        }
    }
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

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        ErrorExit("Connection to server failed");
    }

    std::cout << "Connected to IOCP server!" << std::endl;

    std::thread t1(RecvThread, clientSocket);
    t1.detach();


    std::cout << "이름을 입력해주세요." << std::endl;

    std::string name;
    std::cin >> name;

    C2SSetNamePacket setName;
    strcpy(setName.Name, name.c_str());

    PacketBase setNamePac;
    setNamePac.PacID = C2SSetName;
    setNamePac.PacketSize = sizeof(PacketBase) + sizeof(C2SSetNamePacket);
    memcpy(&setNamePac.Body, &setName, sizeof(C2SSetNamePacket));

    char buffer[BUFFER_SIZE];
    memcpy(buffer, &setNamePac, sizeof(PacketBase) + sizeof(C2SSetNamePacket));

    int bytesSent = send(clientSocket, buffer, setNamePac.PacketSize, 0);
    if (bytesSent == SOCKET_ERROR) 
    {
        ErrorExit("Send failed");
    }

    while (true)
    {
        std::string msg;
        std::getline(std::cin, msg);
        
        if (msg == "e")
        {
            break;
        }
        
        C2SSendMSGPacket sendPac;
        strcpy(sendPac.Name, name.c_str());
        strcpy(sendPac.MSG, msg.c_str());

        PacketBase pac;
        pac.PacID = C2SSendMSG;
        pac.PacketSize = sizeof(PacketBase) + sizeof(C2SSendMSGPacket);
        memcpy(&pac.Body, &sendPac, sizeof(C2SSendMSGPacket));

        char buffer[BUFFER_SIZE];
        memcpy(buffer, &pac, sizeof(PacketBase) + sizeof(C2SSendMSGPacket));

        int bytesSent = send(clientSocket, buffer, pac.PacketSize, 0);
        if (bytesSent == SOCKET_ERROR) 
        {
            ErrorExit("Send failed");
        }
    }

    closesocket(clientSocket);
    WSACleanup();

	return 0;
}