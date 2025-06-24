#include "DBManager.h"

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
    }
    catch (sql::SQLException& e) 
    {
        std::cerr << "[DBManager::Init] DB 연결 실패: " << e.what() << std::endl;
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