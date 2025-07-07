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
    using CheckIDResultHandler = std::function<void(const uint8_t)>;
	using SignUpResultHandler = std::function<void(const uint8_t)>;
	using LoginResultHandler = std::function<void(const uint8_t, const uint16_t, const char*, const std::vector<S2CCharacterInfo>& characters)>;
	using CheckNicknameResultHandler = std::function<void(uint8_t)>;
	using CreateCharacterResultHandler = std::function<void(const uint8_t, const uint16_t, const uint8_t, const char*)>;
	using SelecteCharacterResultHandler = std::function<void(const uint8_t, const uint16_t, const char*, const uint16_t, const uint32_t)>;
	using RankingResultHandler = std::function<void(const uint16_t, const std::vector<S2CRankingInfo>&)>;
    using MapChangeHandler = std::function<void(PacketBase*)>;
    using MonsterStateInfoHandler = std::function<void(const std::vector<S2CMonsterStateInfo>&)>;
    using PlayerEnterHandler = std::function<void(const uint16_t, const char*, const float, const float)>;
    using PlayerLeaveHandler = std::function<void(const uint16_t)>;
    using PlayerInfoHandler = std::function<void(const std::vector<S2CPlayerStateInfo>&)>;
	using OtherPlayerPosSyncHandler = std::function<void(const S2COtherPlayerPosSyncPacket*)>;
	using PlayerPosSyncHandler = std::function<void(const float, const float, const float, const uint32_t)>;
	using MonsterHitInfoHandler = std::function<void(const std::vector<S2CMonsterHitInfo>&)>;
	using MonsterRespawnHandler = std::function<void(const std::vector<S2CMonsterRespawnInfo>&)>;
	using PlayerAttackHandler = std::function<void(const uint16_t, Direction)>;
	using ExpGainHandler = std::function<void(const uint16_t, const uint16_t, const uint16_t)>;

    Network();
    ~Network();

    bool Connect(const std::string& ip, int port);
    void Disconnect();

    void IDCheck(const std::string& name);
	void SignUp(const std::string& name, const std::string& password);
	void Login(const std::string& name, const std::string& password);
	void CharacterNameCheck(const std::string& name);
    void CreateCharacter(const std::string& name, const uint8_t gender);
	void SelectCharacter(const uint16_t characterID);
	void RequestRanking();
    void SetName(const std::string& name);
    void SendChat(const std::string& msg);
    void ChangeMap(unsigned int mapID);
	void ChangeMapByPortal(unsigned int portalID);
    void PlayerMove(Direction dir, uint32_t frameID);
	void PlayerStop(uint32_t frameID);
	void PlayerAttack(Direction dir);
	void PlayerPosSync(float x, float y, float z, uint32_t frameID);

	void SetCheckIDResultCallback(CheckIDResultHandler handler);
	void SetSignUpResultCallback(SignUpResultHandler handler);
	void SetLoginResultCallback(LoginResultHandler handler);
	void SetCheckNicknameResultCallback(CheckNicknameResultHandler handler);
	void SetCreateCharacterResultCallback(CreateCharacterResultHandler handler);
	void SetSelectCharacterResultCallback(SelecteCharacterResultHandler handler);
	void SetRankingResultCallback(RankingResultHandler handler);
    void SetMessageCallback(MessageHandler handler);
    void SetMapChangeCallback(MapChangeHandler handler);
    void SetMonsterInfoCallback(MonsterStateInfoHandler handler);
    void SetPlayerEnterCallback(PlayerEnterHandler handler);
    void SetPlayerLeaveCallback(PlayerLeaveHandler handler);
	void SetPlayerInfoCallback(PlayerInfoHandler handler);
	void SetOtherPlayerPosSyncCallback(OtherPlayerPosSyncHandler handler);
	void SetPlayerPosSyncCallback(PlayerPosSyncHandler handler);
	void SetMonsterHitInfoCallback(MonsterHitInfoHandler handler);
    void SetMonsterRespawnCallback(MonsterRespawnHandler handler);
	void SetPlayerAttackCallback(PlayerAttackHandler handler);
	void SetExpGainCallback(ExpGainHandler handler);

private:
    void RecvLoop();
    void HandlePacket(PacketBase* pac, const char* body);

    SOCKET m_socket;
    std::thread m_recvThread;
    RingBuffer m_ringBuffer;
    bool m_isRunning;

    std::mutex m_mutex;

	CheckIDResultHandler m_checkIDResultHandler;
	SignUpResultHandler m_signUpResultHandler;
	LoginResultHandler m_loginResultHandler;
	CheckNicknameResultHandler m_checkNicknameResultHandler;
	CreateCharacterResultHandler m_createCharacterResultHandler;
	SelecteCharacterResultHandler m_selectCharacterResultHandler;
	RankingResultHandler m_rankingResultHandler;
    MessageHandler m_messageHandler;
    MapChangeHandler m_mapChangeHandler;
    MonsterStateInfoHandler m_monsterInfoHandler;
    PlayerEnterHandler m_playerEnterHandler;
    PlayerLeaveHandler m_playerLeaveHandler;
	PlayerInfoHandler m_playerInfoHandler;
	OtherPlayerPosSyncHandler m_otherPlayerPosSyncHandler;
	PlayerPosSyncHandler m_playerPosSyncHandler;
	MonsterHitInfoHandler m_monsterHitInfoHandler;
    MonsterRespawnHandler m_monsterRespawnHandler;
	PlayerAttackHandler m_playerAttackHandler;
	ExpGainHandler m_expGainHandler;
};