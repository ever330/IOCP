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

void RedisManager::SetSessionMapping(unsigned int userID, unsigned int sessionID) 
{
    redisCommand(m_context, "SET user:%u:session %u", userID, sessionID);
    redisCommand(m_context, "SET session:%u:user %u", sessionID, userID);
}

int RedisManager::GetSessionIDByUserID(unsigned int userID) 
{
    redisReply* reply = (redisReply*)redisCommand(m_context, "GET user:%u:session", userID);
    if (reply && reply->type == REDIS_REPLY_STRING) 
    {
        int result = std::stoi(reply->str);
        freeReplyObject(reply);
        return result;
    }
    freeReplyObject(reply);
    return -1; // 없으면 -1
}

int RedisManager::GetUserIDBySessionID(unsigned int sessionID)
{
    redisReply* reply = (redisReply*)redisCommand(m_context, "GET session:%u:user", sessionID);
    if (reply && reply->type == REDIS_REPLY_STRING) 
    {
        int result = std::stoi(reply->str);
        freeReplyObject(reply);
        return result;
    }
    freeReplyObject(reply);
	return -1; // 없으면 -1
}

void RedisManager::RemoveMapping(unsigned int sessionID) 
{
    redisCommand(m_context, "DEL session:%u:user", sessionID);
    redisCommand(m_context, "DEL user:%u:session", GetUserIDBySessionID(sessionID));
}

RedisManager::~RedisManager() 
{
    if (m_context) 
    {
        redisFree(m_context);
    }
}