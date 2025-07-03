#include "MapManager.h"

void MapManager::Initialize()
{
    // �ʿ��� �� ���
    LoadMaps();

    for (auto& pair : m_maps)
    {
        pair.second->Initialize();
	}
}

void MapManager::Update(float deltaTime, int tickCount)
{  
    std::scoped_lock lock(m_mutex);  
    for (auto& pair : m_maps)  
    {  
        pair.second->Update(deltaTime, tickCount);
    }  
}

std::shared_ptr<Map> MapManager::GetMap(unsigned int mapId)
{
    if (m_maps.find(mapId) == m_maps.end())
    {
        return nullptr; // ���� �������� ������ nullptr ��ȯ
	}
    return m_maps[mapId];
}

void MapManager::LoadMaps()
{
    // ����� �� ������ �ϵ��ڵ����� ������̳� ���� �� �����͸� ���� �ҷ����� ������� ������ ����
    m_maps.emplace(1001, std::make_shared<Map>(1001));
    m_maps[1001]->AddPortal(1002, Vector3(450.0f, 40.0f, 0.0f), Vector3(40.0f, 300.0f, 0.0f));
    m_maps[1001]->AddPortal(1003, Vector3(450.0f, 300.0f, 0.0f), Vector3(40.0f, 40.0f, 0.0f));

    m_maps.emplace(1002, std::make_shared<Map>(1002));
    m_maps[1002]->AddPortal(1001, Vector3(40.0f, 300.0f, 0.0f), Vector3(450.0f, 40.0f, 0.0f));
    m_maps[1002]->AddPortal(1003, Vector3(250.0f, 300.0f, 0.0f), Vector3(250.0f, 40.0f, 0.0f));
    m_maps[1002]->AddPortal(2001, Vector3(450.0f, 170.0f, 0.0f), Vector3(40.0f, 170.0f, 0.0f));

    m_maps.emplace(1003, std::make_shared<Map>(1003));
    m_maps[1003]->AddPortal(1001, Vector3(40.0f, 40.0f, 0.0f), Vector3(450.0f, 300.0f, 0.0f));
    m_maps[1003]->AddPortal(1002, Vector3(250.0f, 40.0f, 0.0f), Vector3(250.0f, 300.0f, 0.0f));
    m_maps[1003]->AddPortal(3001, Vector3(450.0f, 170.0f, 0.0f), Vector3(40.0f, 170.0f, 0.0f));

    m_maps.emplace(2001, std::make_shared<Map>(2001));
	m_maps[2001]->AddPortal(1002, Vector3(40.0f, 170.0f, 0.0f), Vector3(450.0f, 170.0f, 0.0f));
	m_maps[2001]->AddPortal(2002, Vector3(450.0f, 170.0f, 0.0f), Vector3(40.0f, 170.0f, 0.0f));

    m_maps.emplace(2002, std::make_shared<Map>(2002));
	m_maps[2002]->AddPortal(2001, Vector3(40.0f, 170.0f, 0.0f), Vector3(450.0f, 170.0f, 0.0f));
	m_maps[2002]->AddPortal(4001, Vector3(450.0f, 170.0f, 0.0f), Vector3(40.0f, 170.0f, 0.0f));

    m_maps.emplace(3001, std::make_shared<Map>(3001));
	m_maps[3001]->AddPortal(1003, Vector3(40.0f, 170.0f, 0.0f), Vector3(450.0f, 170.0f, 0.0f));
	m_maps[3001]->AddPortal(3002, Vector3(450.0f, 170.0f, 0.0f), Vector3(40.0f, 170.0f, 0.0f));

    m_maps.emplace(3002, std::make_shared<Map>(3002));
	m_maps[3002]->AddPortal(3001, Vector3(40.0f, 170.0f, 0.0f), Vector3(450.0f, 170.0f, 0.0f));
	m_maps[3002]->AddPortal(5001, Vector3(450.0f, 170.0f, 0.0f), Vector3(40.0f, 170.0f, 0.0f));

    m_maps.emplace(4001, std::make_shared<Map>(4001));
	m_maps[4001]->AddPortal(2002, Vector3(40.0f, 170.0f, 0.0f), Vector3(450.0f, 170.0f, 0.0f));
    m_maps[4001]->AddPortal(4002, Vector3(450.0f, 40.0f, 0.0f), Vector3(40.0f, 300.0f, 0.0f));
    m_maps[4001]->AddPortal(4003, Vector3(450.0f, 300.0f, 0.0f), Vector3(40.0f, 40.0f, 0.0f));

    m_maps.emplace(4002, std::make_shared<Map>(4002));
	m_maps[4002]->AddPortal(4001, Vector3(40.0f, 300.0f, 0.0f), Vector3(450.0f, 40.0f, 0.0f));
	m_maps[4002]->AddPortal(4003, Vector3(250.0f, 300.0f, 0.0f), Vector3(250.0f, 40.0f, 0.0f));

    m_maps.emplace(4003, std::make_shared<Map>(4003));
	m_maps[4003]->AddPortal(4001, Vector3(40.0f, 40.0f, 0.0f), Vector3(450.0f, 300.0f, 0.0f));
	m_maps[4003]->AddPortal(4002, Vector3(250.0f, 40.0f, 0.0f), Vector3(250.0f, 300.0f, 0.0f));

    m_maps.emplace(5001, std::make_shared<Map>(5001));
	m_maps[5001]->AddPortal(3002, Vector3(40.0f, 170.0f, 0.0f), Vector3(450.0f, 170.0f, 0.0f));
}
