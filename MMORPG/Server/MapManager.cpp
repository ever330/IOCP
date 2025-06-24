#include "MapManager.h"

void MapManager::Initialize()
{
    // 필요한 맵 등록
    m_maps.emplace(1001, std::make_shared<Map>(1001));
    m_maps.emplace(1002, std::make_shared<Map>(1002));
    m_maps.emplace(1003, std::make_shared<Map>(1003));
    m_maps.emplace(2001, std::make_shared<Map>(2001));
    m_maps.emplace(2002, std::make_shared<Map>(2002));
    m_maps.emplace(3001, std::make_shared<Map>(3001));
    m_maps.emplace(3002, std::make_shared<Map>(3002));
    m_maps.emplace(4001, std::make_shared<Map>(4001));
    m_maps.emplace(4002, std::make_shared<Map>(4002));
    m_maps.emplace(4003, std::make_shared<Map>(4003));
    m_maps.emplace(5001, std::make_shared<Map>(5001));

    for (auto& pair : m_maps)
    {
        pair.second->Initialize();
	}
}

void MapManager::Update()  
{  
    std::scoped_lock lock(m_mutex);  
    for (auto& pair : m_maps)  
    {  
        pair.second->Update();
    }  
}

std::shared_ptr<Map> MapManager::GetMap(unsigned int mapId)
{
    if (m_maps.find(mapId) == m_maps.end())
    {
        return nullptr; // 맵이 존재하지 않으면 nullptr 반환
	}
    return m_maps[mapId];
}
