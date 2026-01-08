#include "pch.h"
#include "Config.h"
#include <fstream>

bool ConfigManager::LoadConfig(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "??쇱젟 ???뵬????????곷뮸??덈뼄: " << filePath << std::endl;
        return false;
    }

    std::string line;
    std::string currentSection;

    while (std::getline(file, line))
    {
        line = Trim(line);

        // ??餓κ쑴???雅뚯눘苑??얜똻??
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        // ?諭?????뼓 [Section]
        if (line[0] == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        // key=value ???뼓
        size_t delimPos = line.find('=');
        if (delimPos == std::string::npos)
            continue;

        std::string key = Trim(line.substr(0, delimPos));
        std::string value = Trim(line.substr(delimPos + 1));

        // 揶???쇱젟
        if (currentSection == "Database")
        {
            if (key == "Host") m_config.dbHost = value;
            else if (key == "User") m_config.dbUser = value;
            else if (key == "Password") m_config.dbPassword = value;
            else if (key == "Schema") m_config.dbSchema = value;
        }
        else if (currentSection == "Redis")
        {
            if (key == "Host") m_config.redisHost = value;
            else if (key == "Port") m_config.redisPort = std::stoi(value);
            else if (key == "Password") m_config.redisPassword = value;
        }
        else if (currentSection == "Server")
        {
            if (key == "Port") m_config.serverPort = std::stoi(value);
        }
    }

    file.close();
    return true;
}

std::string ConfigManager::Trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
        return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}
