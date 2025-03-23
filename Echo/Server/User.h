#pragma once

class User
{
public:
	User() = delete;
	~User() {}
	User(unsigned int netId) : m_netId(netId) {}

public:
	void SetNetId(unsigned int netId) { m_netId = netId; }
	unsigned int GetNetId() { return m_netId; }

private:
	unsigned int m_netId;
};