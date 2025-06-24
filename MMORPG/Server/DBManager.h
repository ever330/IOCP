#pragma once

#include "pch.h"

struct DBRequest
{
    std::string query;
    std::function<void(bool, sql::ResultSet*)> callback;
};

class DBManager
{
public:
    DBManager();
    ~DBManager();

    void Initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& schema);
    void RequestQuery(const std::string& sql, std::function<void(bool, sql::ResultSet*)> callback);

private:
    void DBThreadLoop();
    void Shutdown();

    std::queue<DBRequest> m_requestQueue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::thread m_thread;

    sql::mysql::MySQL_Driver* m_driver = nullptr;
    std::unique_ptr<sql::Connection> m_con;

    bool m_running = true;
};

