#pragma once

#include "pch.h"
#include "RingBuffer.h"
#include "Packet.h"
#include "Define.h"
#include "Util.h"

#pragma comment(lib, "ws2_32.lib")

class Network
{
public:
    using MessageHandler = std::function<void(const std::string&)>;
    using MapChangeHandler = std::function<void(unsigned int, float, float)>;
    using MonsterStateInfoHandler = std::function<void(const std::vector<S2CMonsterStateInfo>&)>;
    using PlayerEnterHandler = std::function<void(const uint16_t, const char*, float, float)>;
    using PlayerLeaveHandler = std::function<void(const uint16_t)>;
    using PlayerInfoHandler = std::function<void(const std::vector<S2CPlayerStateInfo>&)>;
	using MonsterHitInfoHandler = std::function<void(const std::vector<S2CMonsterHitInfo>&)>;
	using MonsterRespawnHandler = std::function<void(const std::vector<S2CMonsterRespawnInfo>&)>;
	using PlayerAttackHandler = std::function<void(const uint16_t, Direction)>;
    using CheckIDResultHandler = std::function<void(uint8_t)>;

    Network();
    ~Network();

    bool Connect(const std::string& ip, int port);
    void Disconnect();

    void IDCheck(const std::string& name);
    void SetName(const std::string& name);
    void SendChat(const std::string& msg);
    void ChangeMap(unsigned int mapID);
    void PlayerMove(Direction dir);
	void PlayerAttack(Direction dir);

	void SetCheckIDResultCallback(CheckIDResultHandler handler);
    void SetMessageCallback(MessageHandler handler);
    void SetMapChangeCallback(MapChangeHandler handler);
    void SetMonsterInfoCallback(MonsterStateInfoHandler handler);
    void SetPlayerEnterCallback(PlayerEnterHandler handler);
    void SetPlayerLeaveCallback(PlayerLeaveHandler handler);
	void SetPlayerInfoCallback(PlayerInfoHandler handler);
	void SetMonsterHitInfoCallback(MonsterHitInfoHandler handler);
    void SetMonsterRespawnCallback(MonsterRespawnHandler handler);
	void SetPlayerAttackCallback(PlayerAttackHandler handler);

private:
    void RecvLoop();
    void HandlePacket(PacketBase* pac, const char* body);

    SOCKET m_socket;
    std::thread m_recvThread;
    RingBuffer m_ringBuffer;
    bool m_isRunning;

    std::mutex m_mutex;

	CheckIDResultHandler m_checkIDResultHandler;
    MessageHandler m_messageHandler;
    MapChangeHandler m_mapChangeHandler;
    MonsterStateInfoHandler m_monsterInfoHandler;
    PlayerEnterHandler m_playerEnterHandler;
    PlayerLeaveHandler m_playerLeaveHandler;
	PlayerInfoHandler m_playerInfoHandler;
	MonsterHitInfoHandler m_monsterHitInfoHandler;
    MonsterRespawnHandler m_monsterRespawnHandler;
	PlayerAttackHandler m_playerAttackHandler;
};