#include "pch.h"
#include "DBManager.h"
#include "MainServer.h"

DBManager::DBManager()
{
}

DBManager::~DBManager()
{
    Shutdown();
}

void DBManager::Initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& schema)
{
    try
    {
        m_driver = sql::mysql::get_mysql_driver_instance();
        m_con.reset(m_driver->connect(host, user, password));
        m_con->setSchema(schema);

        m_running = true;
        m_thread = std::thread(&DBManager::DBThreadLoop, this);

        MainServer::Instance().Log("DBManager 초기화 완료");
    }
    catch (sql::SQLException& e)
    {
        std::string errorMsg = "DB 초기화 실패: " + std::string(e.what());
        MainServer::Instance().Log(errorMsg);
    }
}

void DBManager::RequestQuery(const std::string& sql, std::function<void(bool, sql::ResultSet*)> callback)
{
    DBRequest request;
    request.type = DBRequestType::Query;
    request.query = sql;
    request.callback = callback;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_requestQueue.push(std::move(request));
    }
    m_cond.notify_one();
}

void DBManager::RequestExecute(const std::string& sql)
{
    DBRequest request;
    request.type = DBRequestType::Execute;
    request.query = sql;
    request.callback = nullptr;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_requestQueue.push(std::move(request));
    }
    m_cond.notify_one();
}

void DBManager::UpdateCharacterLevelAndExpAsync(unsigned int characterID, unsigned int level, unsigned long exp)
{
    // PreparedStatement 사용 시 파라미터 바인딩 필요
    std::string query = "UPDATE Characters SET Level = " + std::to_string(level) +
                        ", Exp = " + std::to_string(exp) +
                        " WHERE CharID = " + std::to_string(characterID);

    RequestExecute(query);
}

std::vector<CharacterData> DBManager::GetAllCharacters()
{
    std::vector<CharacterData> result;

    // 동시에 여러 쓰레드가 연결 객체를 사용하면 문제 발생 (DB 작업은 직렬화 필요)
    std::lock_guard<std::mutex> lock(m_conMutex);

    try
    {
        std::string query = "SELECT CharID, Level, Exp, Name FROM Characters";
        std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(query));
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

        while (res->next())
        {
            CharacterData ch;
            ch.CharacterID = res->getInt("CharID");
            ch.Level = res->getInt("Level");
            ch.Experience = res->getUInt64("Exp");
            ch.Name = res->getString("Name");
            result.push_back(std::move(ch));
        }
    }
    catch (sql::SQLException& e)
    {
        MainServer::Instance().Log("GetAllCharacters 실패: " + std::string(e.what()));
    }

    return result;
}

void DBManager::DBThreadLoop()
{
    while (m_running)
    {
        DBRequest request;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [&]() { return !m_requestQueue.empty() || !m_running; });

            if (!m_running && m_requestQueue.empty())
                break;

            if (m_requestQueue.empty())
                continue;

            request = std::move(m_requestQueue.front());
            m_requestQueue.pop();
        }

        std::lock_guard<std::mutex> conLock(m_conMutex);

        try
        {
            std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(request.query));

            if (request.type == DBRequestType::Query)
            {
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

                if (request.callback)
                {
                    // 콜백에서 ResultSet 관리 - 콜백에서 처리 후 자동 해제
                    request.callback(true, res.release());
                }
            }
            else // Execute
            {
                stmt->execute();
            }
        }
        catch (sql::SQLException& e)
        {
            MainServer::Instance().Log("[DBManager] 쿼리 실패: " + std::string(e.what()));
            if (request.callback)
            {
                request.callback(false, nullptr);
            }
        }
    }
}

void DBManager::Shutdown()
{
    m_running = false;
    m_cond.notify_all();
    if (m_thread.joinable())
        m_thread.join();
}
