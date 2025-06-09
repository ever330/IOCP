#pragma once

#include "pch.h"
#include "Map.h"

class MapManager
{
public:
	MapManager() = default;
	~MapManager() = default;

public:
	void Initialize(std::shared_ptr<IOCP> iocp);
	void Update();
	std::shared_ptr<Map> GetMap(unsigned int mapId);

private:
	std::shared_ptr<IOCP> m_IOCP;
	std::mutex m_mutex;
	std::unordered_map<unsigned int, std::shared_ptr<Map>> m_maps;
};

