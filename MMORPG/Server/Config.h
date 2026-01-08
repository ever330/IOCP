#pragma once

#include "pch.h"

struct ServerConfig
{
    // DB ??쇱젟
    std::string dbHost;
    std::string dbUser;
    std::string dbPassword;
    std::string dbSchema;

    // Redis ??쇱젟
    std::string redisHost;
    int redisPort;
    std::string redisPassword;

    // ??뺤쒔 ??쇱젟
    int serverPort;
};

class ConfigManager
{
private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

public:
    static ConfigManager& Instance()
    {
        static ConfigManager instance;
        return instance;
    }

    bool LoadConfig(const std::string& filePath);
    const ServerConfig& GetConfig() const { return m_config; }

private:
    std::string Trim(const std::string& str);

    ServerConfig m_config;
};
