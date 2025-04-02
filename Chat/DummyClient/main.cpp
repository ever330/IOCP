#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include "Packet.h"
#include "Define.h"
#include "RingBuffer.h"
#include <random>

#pragma comment(lib, "ws2_32.lib")

constexpr int PORT = 3030;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int HEADER_SIZE = 4;
constexpr int BODY_SIZE = 1024;
constexpr int BUFFER_SIZE = 1024;
constexpr int THREAD_COUNT = 5;

void ErrorExit(const char* msg)
{
	std::cerr << msg << " Error: " << WSAGetLastError() << std::endl;
	exit(EXIT_FAILURE);
}

std::string GetMsg()
{
	static thread_local std::mt19937 generator(std::random_device{}());
	std::uniform_int_distribution<int> distribution(0, 4);

	std::string messages[] = { "Hello", "Hi", "Bye", "안녕하세요.", "안녕히계세요." };
	return messages[distribution(generator)];
}

void RecvThread(std::vector<SOCKET> sockets, std::vector<RingBuffer>& buffers)
{
	while (true)
	{
		for (int i = 0; i < sockets.size(); i++)
		{
			SOCKET clientSocket = sockets[i];

			char buffer[BUFFER_SIZE];
			int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);

			if (bytesReceived == 0)
			{
				std::cout << "Server closed connection" << std::endl;
				return;
			}
			else if (bytesReceived < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
			{
				ErrorExit("Receive failed");
			}

			buffers[i].Write(buffer, bytesReceived);

			char header[HEADER_SIZE];
			char body[BODY_SIZE];
			if (buffers[i].Read(header, sizeof(PacketBase)))
			{
				PacketBase* pac = reinterpret_cast<PacketBase*>(header);
				if (buffers[i].Read(body, pac->PacketSize - sizeof(PacketBase)))
				{
					if (pac->PacID == S2CSendMSG)
					{
						S2CSendMSGPacket msg;
						memcpy(&msg, body, sizeof(S2CSendMSGPacket));

						int index = 0;
						while (msg.Name[index] && !isdigit(msg.Name[index])) index++;

						int num = atoi(msg.Name + index);

						if (i + 1 == num)
							std::cout << msg.Name << " : " << msg.MSG << std::endl;
					}
					else if (pac->PacID == S2CNewUserAlert)
					{
						S2CNewUserAlertPacket alert;
						memcpy(&alert, body, sizeof(S2CNewUserAlertPacket));

						int index = 0;
						while (alert.Name[index] && !isdigit(alert.Name[index])) index++;

						int num = atoi(alert.Name + index);

						if (i + 1 == num)
							std::cout << alert.Name << " 님이 접속하셨습니다." << std::endl;
					}
				}
			}
		}
	}
}

void SendThread(std::vector<SOCKET> sockets, int start, int end)
{
	while (true)
	{
		for (int i = start; i <= end; i++)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));

			C2SSendMSGPacket sendPac;
			std::string name = "test" + std::to_string(i + 1);
			strcpy(sendPac.Name, name.c_str());
			strcpy(sendPac.MSG, GetMsg().c_str());

			PacketBase pac;
			pac.PacID = C2SSendMSG;
			pac.PacketSize = sizeof(PacketBase) + sizeof(C2SSendMSGPacket);

			char* buffer = new char[pac.PacketSize];

			memcpy(buffer, &pac, sizeof(PacketBase));
			memcpy(buffer + sizeof(PacketBase), &sendPac, sizeof(C2SSendMSGPacket));

			int bytesSent = send(sockets[i], buffer, pac.PacketSize, 0);
			delete[] buffer;
		}
	}
}

int main()
{
	int testCount = 0;

	std::cout << "테스트 연결용 소켓 수를 입력해주세요." << std::endl;
	std::cin >> testCount;

	std::vector<SOCKET> testSockets;
	std::vector<RingBuffer> ringBuffers;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorExit("WSAStartup failed");
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	for (int i = 1; i <= testCount; i++)
	{
		// 테스트 소켓 연결
		SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (clientSocket == INVALID_SOCKET)
		{
			ErrorExit("Socket creation failed");
		}
		if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		{
			ErrorExit("Connection to server failed");
		}
		testSockets.push_back(clientSocket);

		ringBuffers.emplace_back(1024);

		// 테스트 소켓 별 이름 지정
		C2SSetNamePacket setName;
		std::string name = "test" + std::to_string(i);
		strcpy(setName.Name, name.c_str());

		int totalSize = sizeof(PacketBase) + sizeof(C2SSetNamePacket);
		char* buffer = new char[totalSize];

		PacketBase* setNamePac = reinterpret_cast<PacketBase*>(buffer);
		setNamePac->PacID = C2SSetName;
		setNamePac->PacketSize = totalSize;

		// 정확한 위치에 데이터 복사
		memcpy(buffer + sizeof(PacketBase), &setName, sizeof(C2SSetNamePacket));

		// 데이터 전송
		int bytesSent = send(clientSocket, buffer, totalSize, 0);
		if (bytesSent == SOCKET_ERROR)
		{
			ErrorExit("Send failed");
		}

		// 동적 메모리 해제
		delete[] buffer;
	}

	std::thread recvThread(RecvThread, testSockets, std::ref(ringBuffers));
	recvThread.detach();

	for (int i = 0; i < THREAD_COUNT; i++)
	{
		int start = i * (testCount / THREAD_COUNT);
		int end = (i == THREAD_COUNT - 1) ? (testCount - 1) : ((i + 1) * (testCount / THREAD_COUNT) - 1);
		std::thread sendThread(SendThread, testSockets, start, end);
		sendThread.detach();
	}

	while (true)
	{
		char cmd;
		std::cin >> cmd;

		if (cmd == 'e')
			break;
	}

	for (SOCKET clientSocket : testSockets)
	{
		closesocket(clientSocket);
	}
	WSACleanup();

	return 0;
}