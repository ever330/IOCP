#pragma once

#include "pch.h"
#include <hiredis/hiredis.h>

class RedisManager {
public:
    static RedisManager& Instance();

    bool Connect(const std::string& ip, int port, const std::string& password);
    void SetSessionMapping(unsigned int userID, unsigned int sessionID);
    int GetSessionIDByUserID(unsigned int userID);
	int GetUserIDBySessionID(unsigned int sessionID);
    void RemoveMapping(unsigned int sessionID);
    ~RedisManager();

private:
    redisContext* m_context = nullptr;
};