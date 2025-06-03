#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"
#include "Packet.h"
#include "Define.h"
#include "RingBuffer.h"
#include "Vector3.h"

#pragma comment(lib, "ws2_32.lib")

constexpr int PORT = 3030;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int HEADER_SIZE = 4;
constexpr int BODY_SIZE = 1024;
constexpr int BUFFER_SIZE = 1024;
constexpr int THREAD_COUNT = 5;

std::vector<SOCKET> testSockets;
std::vector<Vector3> positions;
std::vector<RingBuffer> ringBuffers;
std::vector<unsigned int> mapIDs = { 1001, 1002, 1003, 2001, 2002, 3001, 3002, 4001, 4002, 4003, 5001 };

std::mutex m_mutex;

void ChangeMap(unsigned int mapID, SOCKET socket);
void SendSetName(SOCKET socket, const std::string& name);
void SendPlayerMove(SOCKET socket, int direction);

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

void RecvWorker()
{
	std::vector<std::string> messages;
	while (true)
	{
		for (int i = 0; i < testSockets.size(); i++)
		{
			SOCKET clientSocket = testSockets[i];

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

			ringBuffers[i].Write(buffer, bytesReceived);

			char header[HEADER_SIZE];
			char body[BODY_SIZE];
			if (ringBuffers[i].Read(header, sizeof(PacketBase)))
			{
				PacketBase* pac = reinterpret_cast<PacketBase*>(header);
				if (ringBuffers[i].Read(body, pac->PacketSize - sizeof(PacketBase)))
				{
					if (pac->PacID == S2CSendMSG)
					{

					}
					else if (pac->PacID == S2CNewUserAlert)
					{

					}
					else if (pac->PacID == S2CChangeMapAck)
					{
						S2CChangeMapAckPacket* changeMapAck = reinterpret_cast<S2CChangeMapAckPacket*>(body);

						if (changeMapAck->Result != 0)
						{
							positions[i].x = changeMapAck->SpawnPosX;
							positions[i].y = changeMapAck->SpawnPosY;
							positions[i].z = changeMapAck->SpawnPosZ;
						}
					}
					else if (pac->PacID == S2CPlayerChat)
					{

					}
					else if (pac->PacID == S2CPlayerEnter)
					{

					}
					else if (pac->PacID == S2CPlayerLeave)
					{

					}
					else if (pac->PacID == S2CMonsterRespawn)
					{

					}
					else if (pac->PacID == S2CMonsterState)
					{

					}
				}
			}
		}
	}
}

void SendWorker(int start, int end)
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		for (int i = start; i <= end; i++)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dis(0, 3);
			int direction = dis(gen);

			if (positions[i].x == 10.0f && direction == 0)
			{
				direction = 1;
			}
			else if (positions[i].x == MAP_MAX_X - 10 && direction == 1)
			{
				direction = 0;
			}
			else if (positions[i].y == 10.0f && direction == 2)
			{
				direction = 3;
			}
			else if (positions[i].y == MAP_MAX_Y - 10 && direction == 3)
			{
				direction = 2;
			}
			SendPlayerMove(testSockets[i], direction);
		}
	}
}

void SendSetName(SOCKET socket, const std::string& name)
{
	C2SSetNamePacket setName;
	strcpy(setName.Name, name.c_str());
	int totalSize = sizeof(PacketBase) + sizeof(C2SSetNamePacket);
	char* buffer = new char[totalSize];
	PacketBase* setNamePac = reinterpret_cast<PacketBase*>(buffer);
	setNamePac->PacID = C2SSetName;
	setNamePac->PacketSize = totalSize;
	memcpy(buffer + sizeof(PacketBase), &setName, sizeof(C2SSetNamePacket));
	int bytesSent = send(socket, buffer, totalSize, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		ErrorExit("Send failed");
	}
	delete[] buffer;
}

void SendPlayerMove(SOCKET socket, int direction)
{
	C2SPlayerMovePacket movePac;
	movePac.MoveDirection = direction;
	
	int totalSize = sizeof(PacketBase) + sizeof(C2SPlayerMovePacket);
	char* buffer = new char[totalSize];
	PacketBase* movePacketBase = reinterpret_cast<PacketBase*>(buffer);
	movePacketBase->PacID = C2SPlayerMove;
	movePacketBase->PacketSize = totalSize;
	memcpy(buffer + sizeof(PacketBase), &movePac, sizeof(C2SPlayerMovePacket));
	int bytesSent = send(socket, buffer, totalSize, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		ErrorExit("Send failed");
	}
	delete[] buffer;
}

void CreateSocket(int socketCount)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorExit("WSAStartup failed");
	}

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	for (int i = 1; i <= socketCount; i++)
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

		positions.emplace_back(0.0f, 0.0f, 0.0f);

		ringBuffers.emplace_back(1024);
	}
}

void ChangeMap(unsigned int mapID, SOCKET socket)
{
	C2SChangeMapPacket changeMap;
	changeMap.MapID = mapID;

	int totalSize = sizeof(PacketBase) + sizeof(C2SChangeMapPacket);
	char* buffer = new char[totalSize];

	PacketBase* changeMapPac = reinterpret_cast<PacketBase*>(buffer);
	changeMapPac->PacID = C2SChangeMap;
	changeMapPac->PacketSize = totalSize;

	memcpy(buffer + sizeof(PacketBase), &changeMap, sizeof(C2SChangeMapPacket));

	int bytesSent = send(socket, buffer, totalSize, 0);
	if (bytesSent == SOCKET_ERROR)
	{
		ErrorExit("Send failed");
	}
}

int main()
{
	int socketCount = 0;
	std::cout << "테스트 소켓 수를 입력해주세요." << std::endl;
	std::cin >> socketCount;

	CreateSocket(socketCount);

	for (int i = 0; i < socketCount; i++)
	{
		std::string name = "test" + std::to_string(i + 1);
		SendSetName(testSockets[i], name);
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, mapIDs.size() - 1);

	for (int i = 0; i < socketCount; i++)
	{
		int mapIndex = dis(gen);
		unsigned int mapID = mapIDs[mapIndex];
		ChangeMap(mapID, testSockets[i]);
	}

	std::thread recvThread(RecvWorker);
	recvThread.detach();

	for (int i = 0; i < THREAD_COUNT; i++)
	{
		int start = i * (socketCount / THREAD_COUNT);
		int end = (i + 1) * (socketCount / THREAD_COUNT) - 1;
		if (i == THREAD_COUNT - 1)
			end = socketCount - 1;
		std::thread sendThread(SendWorker, start, end);
		sendThread.detach();
	}

	while (true)
	{
		char input;
		std::cin >> input;

		if (input == 'e')
			break;
	}

	for (SOCKET clientSocket : testSockets)
	{
		closesocket(clientSocket);
	}
	WSACleanup();

	return 0;
}