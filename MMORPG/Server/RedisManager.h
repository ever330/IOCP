#pragma once

#include "pch.h"
#include "Packet.h"

class RedisManager 
{
private:
	RedisManager() {}
	~RedisManager() {}

	RedisManager(const RedisManager&) = delete;
	RedisManager& operator=(const RedisManager&) = delete;
	RedisManager(RedisManager&&) = delete;
	RedisManager& operator=(RedisManager&&) = delete;

public:
    static RedisManager& Instance();

    bool Connect(const std::string& ip, int port, const std::string& password);
	void Finalize();
    void SetSessionMapping(unsigned int userID, unsigned int sessionID);
    int GetSessionIDByUserID(unsigned int userID);
	int GetUserIDBySessionID(unsigned int sessionID);
    void RemoveMapping(unsigned int sessionID);
	void UpdateCharacterToRedis(int charID, int level, uint64_t exp, const std::string& name);
	std::vector<S2CRankingInfo> GetTopRankingList(int count);
	unsigned int GetCharacterRanking(int charID);

private:
	int GetIntValue(const std::string& key);
	bool SetIntValue(const std::string& key, int value);
	bool DeleteKey(const std::string& key);

    redisContext* m_context = nullptr;
	std::mutex m_mutex;
};