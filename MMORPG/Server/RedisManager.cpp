#include "RedisManager.h"

RedisManager& RedisManager::Instance()
{
	static RedisManager instance;
	return instance;
}

bool RedisManager::Connect(const std::string& ip, int port, const std::string& password)
{
	m_context = redisConnect(ip.c_str(), port);
	if (!m_context || m_context->err)
	{
		std::cerr << "Redis 연결 실패: " << (m_context ? m_context->errstr : "NULL") << std::endl;
		return false;
	}

	// 비밀번호 인증
	redisReply* reply = (redisReply*)redisCommand(m_context, "AUTH %s", password.c_str());
	if (!reply || m_context->err)
	{
		std::cerr << "Redis 인증 실패: " << m_context->errstr << std::endl;
		redisFree(m_context);
		m_context = nullptr;
		return false;
	}
	freeReplyObject(reply);

	return true;
}

void RedisManager::Finalize()
{
	if (m_context)
	{
		redisFree(m_context);
		m_context = nullptr;
	}
}

void RedisManager::SetSessionMapping(unsigned int userID, unsigned int sessionID)
{
	SetIntValue("user:" + std::to_string(userID), sessionID);
	SetIntValue("session:" + std::to_string(sessionID), userID);
}

int RedisManager::GetSessionIDByUserID(unsigned int userID)
{
	return GetIntValue("user:" + std::to_string(userID));
}

int RedisManager::GetUserIDBySessionID(unsigned int sessionID)
{
	return GetIntValue("session:" + std::to_string(sessionID));
}

void RedisManager::RemoveMapping(unsigned int sessionID)
{
	DeleteKey("user:" + std::to_string(GetUserIDBySessionID(sessionID)));
	DeleteKey("session:" + std::to_string(sessionID));
}

void RedisManager::UpdateCharacterToRedis(int charID, int level, uint64_t exp, const std::string& name)
{
	// 점수 계산
	uint64_t score = (uint64_t)level * 10000000000ULL + exp;

	// ZSET: 랭킹 등록
	redisReply* r1 = (redisReply*)redisCommand(m_context,
		"ZADD CharacterRanking %llu %d", score, charID);
	freeReplyObject(r1);

	// HASH: 캐릭터 정보 저장
	redisReply* r2 = (redisReply*)redisCommand(m_context,
		"HMSET CharacterInfo:%d Level %d Exp %llu Name %s",
		charID, level, exp, name.c_str());
	freeReplyObject(r2);
}

std::vector<S2CRankingInfo> RedisManager::GetTopRankingList(int count)
{
	std::vector<S2CRankingInfo> result;

	redisReply* reply = (redisReply*)redisCommand(m_context, "ZREVRANGE CharacterRanking 0 %d", count - 1);
	if (reply == nullptr || reply->type != REDIS_REPLY_ARRAY)
		return result;

	for (size_t i = 0; i < reply->elements; ++i)
	{
		int charID = std::stoi(reply->element[i]->str);

		redisReply* info = (redisReply*)redisCommand(m_context,
			"HMGET CharacterInfo:%d Level Exp Name", charID);

		if (info && info->type == REDIS_REPLY_ARRAY && info->elements == 3)
		{
			S2CRankingInfo ranker{};
			ranker.CharacterID = charID;
			ranker.Level = std::stoi(info->element[0]->str);
			strncpy_s(ranker.Name, info->element[2]->str, sizeof(ranker.Name) - 1);

			result.push_back(ranker);
		}

		freeReplyObject(info);
	}

	freeReplyObject(reply);
	return result;
}

unsigned int RedisManager::GetCharacterRanking(int charID)
{
	redisReply* reply = (redisReply*)redisCommand(m_context,
		"ZREVRANK CharacterRanking %d", charID);

	int rank = -1;
	if (reply && reply->type == REDIS_REPLY_INTEGER)
		rank = (int)reply->integer;
	rank++;
	freeReplyObject(reply);
	return rank;
}

int RedisManager::GetIntValue(const std::string& key)
{
	std::scoped_lock lock(m_mutex);
	redisReply* reply = (redisReply*)redisCommand(m_context, "GET %s", key.c_str());

	if (reply && reply->type == REDIS_REPLY_STRING)
	{
		int result = std::stoi(reply->str);
		freeReplyObject(reply);
		return result;
	}

	if (reply)
		freeReplyObject(reply);

	return -1;
}

bool RedisManager::SetIntValue(const std::string& key, int value)
{
	std::scoped_lock lock(m_mutex);
	redisReply* reply = (redisReply*)redisCommand(m_context, "SET %s %d", key.c_str(), value);
	bool success = (reply && reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK");

	if (reply)
		freeReplyObject(reply);

	return success;
}

bool RedisManager::DeleteKey(const std::string& key)
{
	std::scoped_lock lock(m_mutex);
	redisReply* reply = (redisReply*)redisCommand(m_context, "DEL %s", key.c_str());
	bool success = (reply && reply->type == REDIS_REPLY_INTEGER && reply->integer > 0);
	if (reply)
		freeReplyObject(reply);
	return success;
}
