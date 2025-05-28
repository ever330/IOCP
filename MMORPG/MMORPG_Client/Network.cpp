#include "Network.h"

Network::Network()
    : m_socket(INVALID_SOCKET), m_ringBuffer(1024)
{
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    m_isRunning = false;
}

Network::~Network()
{
    Disconnect();
    WSACleanup();
}

bool Network::Connect(const std::string& ip, int port)
{
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return false;
    }

    m_isRunning = true;
    m_recvThread = std::thread(&Network::RecvLoop, this);
    return true;
}

void Network::Disconnect()
{
    if (m_socket != INVALID_SOCKET)
    {
        shutdown(m_socket, SD_BOTH);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    m_isRunning = false;
    if (m_recvThread.joinable())
        m_recvThread.join();
}

void Network::SetName(const std::string& name)
{
    C2SSetNamePacket setName;
    strcpy(setName.Name, name.c_str());

    PacketBase pac;
    pac.PacID = C2SSetName;
    pac.PacketSize = sizeof(PacketBase) + sizeof(setName);

    char* buffer = new char[pac.PacketSize];
    memcpy(buffer, &pac, sizeof(pac));
    memcpy(buffer + sizeof(pac), &setName, sizeof(setName));

    send(m_socket, buffer, pac.PacketSize, 0);
    delete[] buffer;
}

void Network::SendChat(const std::string& msg)
{
    C2SPlayerChatPacket chat;
    strcpy(chat.ChatMsg, msg.c_str());

    PacketBase pac;
    pac.PacID = C2SPlayerChat;
    pac.PacketSize = sizeof(PacketBase) + sizeof(chat);

    char* buffer = new char[pac.PacketSize];
    memcpy(buffer, &pac, sizeof(pac));
    memcpy(buffer + sizeof(pac), &chat, sizeof(chat));

    send(m_socket, buffer, pac.PacketSize, 0);
    delete[] buffer;
}

void Network::ChangeMap(unsigned int mapID)
{
    C2SChangeMapPacket changeMap;
    changeMap.MapID = mapID;

    PacketBase pac;
    pac.PacID = C2SChangeMap;
    pac.PacketSize = sizeof(PacketBase) + sizeof(changeMap);

    char* buffer = new char[pac.PacketSize];
    memcpy(buffer, &pac, sizeof(pac));
    memcpy(buffer + sizeof(pac), &changeMap, sizeof(changeMap));

    send(m_socket, buffer, pac.PacketSize, 0);
    delete[] buffer;
}

void Network::PlayerMove(Direction dir)
{
    C2SPlayerMovePacket playerMove;
    playerMove.MoveDirection = (uint8_t)dir;

    PacketBase pac;
    pac.PacID = C2SPlayerMove;
    pac.PacketSize = sizeof(PacketBase) + sizeof(playerMove);

    char* buffer = new char[pac.PacketSize];
    memcpy(buffer, &pac, sizeof(pac));
    memcpy(buffer + sizeof(pac), &playerMove, sizeof(playerMove));

    send(m_socket, buffer, pac.PacketSize, 0);
    delete[] buffer;
}

void Network::PlayerAttack(Direction dir)
{
	C2SPlayerAttackPacket playerAttack;
    playerAttack.AttackDirection = (uint8_t)dir;

    PacketBase pac;
    pac.PacID = C2SPlayerAttack;
    pac.PacketSize = sizeof(PacketBase) + sizeof(playerAttack);
    char* buffer = new char[pac.PacketSize];
    memcpy(buffer, &pac, sizeof(pac));
    memcpy(buffer + sizeof(pac), &playerAttack, sizeof(playerAttack));

    send(m_socket, buffer, pac.PacketSize, 0);
	delete[] buffer;
}

void Network::RecvLoop()
{
    char tempBuf[1024];
    while (m_isRunning)
    {
        int recvLen = recv(m_socket, tempBuf, sizeof(tempBuf), 0);
        if (recvLen <= 0) break;

        m_ringBuffer.Write(tempBuf, recvLen);

        while (true)
        {
            char header[sizeof(PacketBase)];
            if (!m_ringBuffer.Peek(header, sizeof(header)))
                break;

            PacketBase* pac = reinterpret_cast<PacketBase*>(header);
            if (m_ringBuffer.DataSize() < pac->PacketSize)
                break;

            // 패킷 전체 읽기
            std::vector<char> packetBuf(pac->PacketSize);
            if (!m_ringBuffer.Read(packetBuf.data(), pac->PacketSize))
                break;

            PacketBase* pacRead = reinterpret_cast<PacketBase*>(packetBuf.data());
            char* body = packetBuf.data() + sizeof(PacketBase);

            HandlePacket(pacRead, body);
        }
    }
}

void Network::HandlePacket(PacketBase* pac, const char* body)
{
    if (pac->PacID == S2CPlayerChat)
    {
        S2CPlayerChatPacket data;
        memcpy(&data, body, sizeof(data));
        std::string name(data.Name, strnlen(data.Name, NAME_SIZE));
        std::string msg(data.ChatMsg, strnlen(data.ChatMsg, MSG_SIZE));
        if (m_messageHandler)
            m_messageHandler(name + ": " + msg);
    }
    else if (pac->PacID == S2CChangeMapAck)
    {
        S2CChangeMapAckPacket data;
        memcpy(&data, body, sizeof(data));
        if (m_mapChangeHandler)
            m_mapChangeHandler(data.MapID, data.SpawnPosX, data.SpawnPosY);
    }
    else if (pac->PacID == S2CMonsterState)
    {
        const S2CMonsterStatePacket* state = reinterpret_cast<const S2CMonsterStatePacket*>(body);
        const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CMonsterStatePacket) + sizeof(S2CMonsterStateInfo) * state->MonsterCount;

        if (pac->PacketSize != expectedSize)
        {
            // 유효하지 않은 패킷, 로그 출력하거나 무시
			std::cout << "패킷 에러" << std::endl;
            return;
        }

        std::vector<S2CMonsterStateInfo> monsters(state->MonsterCount);
        memcpy(monsters.data(), body + sizeof(S2CMonsterStatePacket), sizeof(S2CMonsterStateInfo) * state->MonsterCount);

        if (m_monsterInfoHandler)
            m_monsterInfoHandler(monsters);
    }
    else if (pac->PacID == S2CPlayerEnter)
    {
        S2CPlayerEnterPacket playerEnter;
        memcpy(&playerEnter, body, sizeof(playerEnter));
        if (m_playerEnterHandler)
            m_playerEnterHandler(playerEnter.UserID, playerEnter.Name, playerEnter.SpawnPosX, playerEnter.SpawnPosY);
    }
    else if (pac->PacID == S2CPlayerLeave)
    {
        S2CPlayerLeavePacket playerLeave;
        memcpy(&playerLeave, body, sizeof(playerLeave));
        if (m_playerLeaveHandler)
            m_playerLeaveHandler(playerLeave.UserID);
    }
    else if (pac->PacID == S2CPlayerState)
    {
        const S2CPlayerStatePacket* state = reinterpret_cast<const S2CPlayerStatePacket*>(body);
        const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CPlayerStatePacket) + sizeof(S2CPlayerStateInfo) * state->PlayerCount;

        if (pac->PacketSize != expectedSize)
        {
            // 유효하지 않은 패킷, 로그 출력하거나 무시
            std::cout << "패킷 에러" << std::endl;
            return;
        }

        std::vector<S2CPlayerStateInfo> users(state->PlayerCount);
        memcpy(users.data(), body + sizeof(S2CPlayerStatePacket), sizeof(S2CPlayerStateInfo) * state->PlayerCount);

        if (m_playerInfoHandler)
            m_playerInfoHandler(users);
    }
    else if (pac->PacID == S2CMonsterHit)
    {
        const S2CMonsterHitPacket* hitInfo = reinterpret_cast<const S2CMonsterHitPacket*>(body);
        const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CMonsterHitPacket) + sizeof(S2CMonsterHitInfo) * hitInfo->MonsterCount;

        if (hitInfo->MonsterCount == 0)
        {
            // 몬스터가 없을 경우, 아무것도 하지 않음
            return;
		}

        if (pac->PacketSize != expectedSize)
        {
            // 유효하지 않은 패킷, 로그 출력하거나 무시
            std::cout << "패킷 에러" << std::endl;
            return;
        }

        std::vector<S2CMonsterHitInfo> hits(hitInfo->MonsterCount);
        memcpy(hits.data(), body + sizeof(S2CMonsterHitPacket), sizeof(S2CMonsterHitInfo) * hitInfo->MonsterCount);

        if (m_monsterHitInfoHandler)
            m_monsterHitInfoHandler(hits);
	}
    else if (pac->PacID == S2CMonsterRespawn)
    {
        const S2CMonsterRespawnPacket* respawnInfo = reinterpret_cast<const S2CMonsterRespawnPacket*>(body);
        const size_t expectedSize = sizeof(PacketBase) + sizeof(S2CMonsterRespawnPacket) + sizeof(S2CMonsterRespawnInfo) * respawnInfo->MonsterCount;

        if (respawnInfo->MonsterCount == 0)
        {
            // 몬스터가 없을 경우, 아무것도 하지 않음
            return;
        }
        if (pac->PacketSize != expectedSize)
        {
            // 유효하지 않은 패킷, 로그 출력하거나 무시
            std::cout << "패킷 에러" << std::endl;
            return;
        }

        std::vector<S2CMonsterRespawnInfo> respawns(respawnInfo->MonsterCount);
        memcpy(respawns.data(), body + sizeof(S2CMonsterRespawnPacket), sizeof(S2CMonsterRespawnInfo) * respawnInfo->MonsterCount);

        if (m_monsterRespawnHandler)
			m_monsterRespawnHandler(respawns);
    }
    else if (pac->PacID == S2CPlayerAttack)
    {
        const S2CPlayerAttackPacket* attackInfo = reinterpret_cast<const S2CPlayerAttackPacket*>(body);
        if (m_playerAttackHandler)
            m_playerAttackHandler(attackInfo->UserID, (Direction)attackInfo->AttackDirection);
    }
    else
    {
        // 알 수 없는 패킷, 로그 출력하거나 무시
        std::cout << "알 수 없는 패킷 ID: " << pac->PacID << std::endl;
	}
}

void Network::SetMessageCallback(MessageHandler handler)
{
    m_messageHandler = handler;
}

void Network::SetMapChangeCallback(MapChangeHandler handler)
{
    m_mapChangeHandler = handler;
}

void Network::SetMonsterInfoCallback(MonsterStateInfoHandler handler)
{
    m_monsterInfoHandler = handler;
}

void Network::SetPlayerEnterCallback(PlayerEnterHandler handler)
{
    m_playerEnterHandler = handler;
}

void Network::SetPlayerLeaveCallback(PlayerLeaveHandler handler)
{
    m_playerLeaveHandler = handler;
}

void Network::SetPlayerInfoCallback(PlayerInfoHandler handler)
{
    m_playerInfoHandler = handler;
}

void Network::SetMonsterHitInfoCallback(MonsterHitInfoHandler handler)
{
	m_monsterHitInfoHandler = handler;
}

void Network::SetMonsterRespawnCallback(MonsterRespawnHandler handler)
{
	m_monsterRespawnHandler = handler;
}

void Network::SetPlayerAttackCallback(PlayerAttackHandler handler)
{
	m_playerAttackHandler = handler;
}
