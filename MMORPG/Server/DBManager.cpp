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

        m_thread = std::thread(&DBManager::DBThreadLoop, this);

		MainServer::Instance().Log("DBManager 초기화 완료!");
    }
    catch (sql::SQLException& e) 
    {
		std::string errorMsg = "DB 연결 실패: " + std::string(e.what());
        MainServer::Instance().Log(errorMsg);
    }
}

void DBManager::RequestQuery(const std::string& sql, std::function<void(bool, sql::ResultSet*)> callback) 
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_requestQueue.push({ sql, callback });
    }
    m_cond.notify_one();
}

void DBManager::UpdateCharacterLevelAndExp(unsigned int characterID, unsigned int level, unsigned long exp)
{
    std::string query = "UPDATE Characters SET Level = ?, Exp = ? WHERE CharID = ?";
    std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(query));
    stmt->setInt(1, level);
    stmt->setUInt(2, exp);
    stmt->setInt(3, characterID);
    stmt->execute();
}

std::vector<CharacterData> DBManager::GetAllCharacters()
{
    std::vector<CharacterData> result;

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
        std::cerr << "GetAllCharacters 오류: " << e.what() << std::endl;
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

            if (!m_running) break;
            request = m_requestQueue.front();
            m_requestQueue.pop();
        }

        try 
        {
            std::unique_ptr<sql::PreparedStatement> stmt(m_con->prepareStatement(request.query));
            std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());

            if (request.callback) 
            {
                request.callback(true, res.release()); // 호출자 측에서 delete 필요
            }
        }
        catch (sql::SQLException& e) 
        {
            std::cerr << "[DBManager] 쿼리 실패: " << e.what() << std::endl;
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
    if (m_thread.joinable()) m_thread.join();
}