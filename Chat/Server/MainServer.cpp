#include "MainServer.h"
#include "IOCP.h"
#include "User.h"
#include "Packet.h"

bool MainServer::StartServer()
{
	m_IOCP = std::make_shared<IOCP>();

	m_IOCP->Initialize();

	m_nextID = 1;

	while (true)
	{
		if (!m_packets.empty())
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			PacketProcess(m_packets.front().first, m_packets.front().second);
			m_packets.pop();
		}
	}

	StopServer();

	return false;
}

void MainServer::PushData(unsigned int sessionID, char* data)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	// ������ �����Ϳ��� ��Ŷ ũ�� �о����
	PacketBase* header = reinterpret_cast<PacketBase*>(data);
	int packetSize = header->PacketSize;

	// ��Ŷ ũ�⸸ŭ ���� �Ҵ�
	PacketBase* pac = reinterpret_cast<PacketBase*>(new char[packetSize]);

	// ��ü ������ ���� (PacketBase + Body ����)
	memcpy(pac, data, packetSize);

	// ����� ���
	std::cout << "SessionID : " << sessionID
		<< "\n ���� ��Ŷ ũ��: " << pac->PacketSize
		<< ", Ÿ��: " << pac->PacID << std::endl;

	unsigned int curUserID = 0;
	if (m_sessionToUserMap.find(sessionID) != m_sessionToUserMap.end())
	{
		curUserID = m_sessionToUserMap[sessionID];
	}
	else
	{
		curUserID = GenerateUserID();
	}

	// ��Ŷ ť�� ����
	m_packets.push(std::make_pair(curUserID, pac));
}

void MainServer::StopServer()
{
	m_IOCP->Finalize();
}

void MainServer::DisconnectClient(unsigned int sessionID)
{
	unsigned int curUserID = 0;
	if (m_sessionToUserMap.find(sessionID) != m_sessionToUserMap.end())
	{
		curUserID = m_sessionToUserMap[sessionID];
		delete m_users[curUserID];
		m_users.erase(curUserID);
	}
	else
	{
		std::cout << "Disconnect Error!" << std::endl;
	}
}

void MainServer::PacketProcess(unsigned int userID, PacketBase* pac)
{
	PacketBase* newPac = nullptr;
	char buffer[BUFFER_SIZE];
	User* newUser = nullptr;

	switch (pac->PacID)
	{
		case C2SSetName:
			C2SSetNamePacket setName;
			memcpy(&setName, pac->Body, pac->PacketSize - sizeof(PacketBase));
			std::cout <<"�̸� ���� : " << setName.Name << std::endl;

			newUser = new User(userID, setName.Name);

			m_users.insert({ newUser->GetUserID(), newUser });

			newPac = reinterpret_cast<PacketBase*>(new char[pac->PacketSize]);
			newPac->PacID = S2CNewUserAlert;
			newPac->PacketSize = pac->PacketSize;

			memcpy(newPac->Body, &setName, newPac->PacketSize - sizeof(PacketBase));
			memcpy(buffer, newPac, newPac->PacketSize);

			m_IOCP->BroadCast(buffer, pac->PacketSize);
			delete[] reinterpret_cast<char*>(newPac);
			break;

		case C2SSendMSG:
			C2SSendMSGPacket msg;
			memcpy(&msg, pac->Body, pac->PacketSize - sizeof(PacketBase));
			std::cout << msg.Name << " : " << msg.MSG << std::endl;
			memcpy(&pac->Body, &msg, sizeof(C2SSendMSGPacket));

			newPac = reinterpret_cast<PacketBase*>(new char[pac->PacketSize]);
			newPac->PacID = S2CSendMSG;
			newPac->PacketSize = pac->PacketSize;

			memcpy(newPac->Body, &msg, newPac->PacketSize - sizeof(PacketBase));

			memcpy(buffer, newPac, newPac->PacketSize);

			m_IOCP->BroadCast(buffer, pac->PacketSize);
			delete[] reinterpret_cast<char*>(newPac);
			break;

		default:
			break;
	}
	delete[] reinterpret_cast<char*>(pac);
}

// IOCP�� SessionID�ʹ� ������ ���� ���� �ο� ID
unsigned int MainServer::GenerateUserID()
{
	// �ӽ�
	return m_nextID++;
}
