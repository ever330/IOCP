#pragma once

#include "pch.h"
#include "Map.h"

class MapManager
{
public:
	MapManager() = default;
	~MapManager() = default;

	void Initialize();
	void Update(float deltaTime, int tickCount);
	std::shared_ptr<Map> GetMap(unsigned int mapId);

private:
	void LoadMaps();

	std::mutex m_mutex;
	std::unordered_map<unsigned int, std::shared_ptr<Map>> m_maps;
};

