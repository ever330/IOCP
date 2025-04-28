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
#include <random>
#include <format>
#include <sstream> 
#include "Packet.h"
#include "Define.h"
#include "RingBuffer.h"

#pragma comment(lib, "ws2_32.lib")

constexpr int PORT = 3030;
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int HEADER_SIZE = 4;
constexpr int BODY_SIZE = 1024;
constexpr int BUFFER_SIZE = 1024;
constexpr int THREAD_COUNT = 5;
const int COMMAND_START_LINE = 22;  // 22번째 줄부터 채팅창
const int COMMAND_START = 14;
const int TOTAL_LINES = 30;          // 콘솔 총 높이
const int GAME_MESSAGE_LINES = COMMAND_START_LINE - 2; // 게임 메시지 표시 줄 수

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

std::vector<SOCKET> testSockets;
std::vector<RingBuffer> ringBuffers;

std::mutex m_mutex;

void CreateSocket(int socketCount);
void RecvStart(int socketCount);
void ChangeMap(unsigned int mapID, SOCKET socket);

void SetCursorPosition(int x, int y)
{
	COORD pos = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(hConsole, pos);
}

void ClearLine(int y)
{
	DWORD written;
	FillConsoleOutputCharacter(hConsole, ' ', 80, { 0, (SHORT)y }, &written);
	FillConsoleOutputAttribute(hConsole, COMMAND_START, 80, { 0, (SHORT)y }, &written);
}

void DrawCommandBox()
{
	// 명령어 박스 위에 구분선 그리기
	SetCursorPosition(0, COMMAND_START_LINE - 1);
	std::cout << std::string(80, '-');

	// 정보
	SetCursorPosition(0, COMMAND_START_LINE);
	std::cout << "맵정보: ";

	// 정보
	SetCursorPosition(0, COMMAND_START_LINE + 1);
	std::cout << "몬스터 정보: ";

	// 명령어 입력 칸
	SetCursorPosition(0, COMMAND_START_LINE + 2);
	std::cout << "명령어 입력: ";
}

void RefreshGameMessages(const std::vector<std::string>& messages)
{
	for (int i = 0; i < GAME_MESSAGE_LINES; ++i)
	{
		ClearLine(i);
	}

	int start = (messages.size() > GAME_MESSAGE_LINES) ? (messages.size() - GAME_MESSAGE_LINES) : 0;
	int line = 0;
	for (size_t i = start; i < messages.size(); ++i)
	{
		SetCursorPosition(0, line++);
		std::cout << messages[i];
	}
}

void CommandInputLoop()
{
	std::vector<std::string> messages;
	std::string input;

	messages.push_back("닉네임을 입력해주세요.");
	RefreshGameMessages(messages);
	SetCursorPosition(COMMAND_START, COMMAND_START_LINE + 2);
	CreateSocket(1);

	while (true)
	{
		SetCursorPosition(COMMAND_START, COMMAND_START_LINE + 2);
		std::cout << std::string(80 - COMMAND_START, ' ');
		SetCursorPosition(COMMAND_START, COMMAND_START_LINE + 2);

		std::getline(std::cin, input);

		if (input.substr(0, 1) == "/")
		{
			if (input.substr(1, 4) == "help")
			{
				messages.push_back("명령어 목록:");
				messages.push_back("/help - 도움말");
				messages.push_back("/exit - 종료");
				messages.push_back("/map [mapID] - 맵 변경");
				RefreshGameMessages(messages);
			}
			else if (input.substr(1, 4) == "exit")
			{
				break;
			}
			else if (input.substr(1, 3) == "map")
			{
				int mapID = std::stoi(input.substr(5));
				ChangeMap(mapID, testSockets[0]);
			}
		}
		else
		{
			// 채팅 메시지 전송
			C2SPlayerChatPacket sendPac;
			strcpy(sendPac.ChatMsg, input.c_str());
			PacketBase pac;
			pac.PacID = C2SPlayerChat;
			pac.PacketSize = sizeof(PacketBase) + sizeof(C2SPlayerChatPacket);
			char* buffer = new char[pac.PacketSize];
			memcpy(buffer, &pac, sizeof(PacketBase));
			memcpy(buffer + sizeof(PacketBase), &sendPac, sizeof(C2SPlayerChatPacket));
			int bytesSent = send(testSockets[0], buffer, pac.PacketSize, 0);
			delete[] buffer;
		}

		// 채팅 입력을 게임 메시지에 추가
		//messages.push_back("[채팅] " + input);

		// 게임 메시지 리프레시 (스크롤)
		//RefreshGameMessages(messages);
	}
}

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

void RecvWorker(std::vector<SOCKET> sockets, std::vector<RingBuffer>& buffers)
{
	std::vector<std::string> messages;
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
						/*S2CNewUserAlertPacket alert;
						memcpy(&alert, body, sizeof(S2CNewUserAlertPacket));

						int index = 0;
						while (alert.Name[index] && !isdigit(alert.Name[index])) index++;

						int num = atoi(alert.Name + index);

						if (i + 1 == num)
							std::cout << alert.Name << " 님이 접속하셨습니다." << std::endl;*/
					}
					else if (pac->PacID == S2CChangeMapAck)
					{
						S2CChangeMapAckPacket changeMapAck;
						memcpy(&changeMapAck, body, sizeof(S2CChangeMapAckPacket));

						std::lock_guard<std::mutex> lock(m_mutex);
						if (changeMapAck.Result == 0)
						{
							messages.push_back("맵 이동 요청 실패");
						}
						else
						{
							messages.push_back("맵 이동 요청 성공");
							SetCursorPosition(10, COMMAND_START_LINE);
							std::cout << "현재위치 " + std::to_string(changeMapAck.MapID) + "(" + std::to_string(changeMapAck.SpawnPosX) + ", " + std::to_string(changeMapAck.SpawnPosY) + ")";
						}
					}
					else if (pac->PacID == S2CPlayerChat)
					{
						S2CPlayerChatPacket chat;
						memcpy(&chat, body, sizeof(S2CPlayerChatPacket));
						std::string name(chat.Name, strnlen(chat.Name, NAME_SIZE));
						std::string msg(chat.ChatMsg, strnlen(chat.ChatMsg, MSG_SIZE));
						messages.push_back(name + " : " + msg);
					}
					else if (pac->PacID == S2CPlayerEnter)
					{
						S2CPlayerEnterPacket enter;
						memcpy(&enter, body, sizeof(S2CPlayerEnterPacket));
						messages.push_back(enter.Name + std::string("님이 접속하셨습니다."));
					}
					else if (pac->PacID == S2CPlayerLeave)
					{
						S2CPlayerLeavePacket leave;
						memcpy(&leave, body, sizeof(S2CPlayerLeavePacket));
						messages.push_back(leave.Name + std::string("님이 퇴장하셨습니다."));
					}
					else if (pac->PacID == S2CMonsterRespawn)
					{
						S2CMonsterRespawnPacket respawn;
						memcpy(&respawn, body, sizeof(S2CMonsterRespawnPacket));
						messages.push_back(std::to_string(respawn.MonsterCount) + std::string("마리의 몬스터가 리스폰되었습니다."));
					}
					else if (pac->PacID == S2CMonsterState)
					{
						S2CMonsterStatePacket state;
						memcpy(&state, body, sizeof(S2CMonsterStatePacket));

						std::vector<S2CMonsterStateInfo> monsterStates(state.MonsterCount);
						memcpy(monsterStates.data(), body + sizeof(S2CMonsterStatePacket), sizeof(S2CMonsterStateInfo) * state.MonsterCount);
						SetCursorPosition(14, COMMAND_START_LINE + 1);
						std::string monsterInfo = "";
						for (const auto& monster : monsterStates)
						{
							std::ostringstream oss;
							oss.precision(1);
							oss << std::fixed << monster.SpawnID << "(" << monster.PosX << "," << monster.PosY << ") ";
							monsterInfo += oss.str();
						}
						std::cout << monsterInfo << std::endl;
					}
				}
				RefreshGameMessages(messages);
				SetCursorPosition(COMMAND_START, COMMAND_START_LINE + 2);
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

		ringBuffers.emplace_back(1024);

		std::string name;
		std::getline(std::cin, name);

		// 테스트 소켓 별 이름 지정
		C2SSetNamePacket setName;
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

	RecvStart(socketCount);
}

void RecvStart(int socketCount)
{
	std::thread recvThread(RecvWorker, testSockets, std::ref(ringBuffers));
	recvThread.detach();

	/*for (int i = 0; i < THREAD_COUNT; i++)
	{
		int start = i * (socketCount / THREAD_COUNT);
		int end = (i == THREAD_COUNT - 1) ? (socketCount - 1) : ((i + 1) * (socketCount / THREAD_COUNT) - 1);
		std::thread sendThread(SendThread, testSockets, start, end);
		sendThread.detach();
	}*/
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
	system("cls");
	DrawCommandBox();
	CommandInputLoop();

	for (SOCKET clientSocket : testSockets)
	{
		closesocket(clientSocket);
	}
	WSACleanup();

	return 0;
}