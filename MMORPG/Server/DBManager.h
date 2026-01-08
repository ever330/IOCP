#pragma once

#include "pch.h"
#include "CharacterData.h"

enum class DBRequestType
{
    Query,      // SELECT (결과값 반환)
    Execute     // INSERT/UPDATE/DELETE (결과값 없음)
};

struct DBRequest
{
    DBRequestType type = DBRequestType::Query;
    std::string query;
    std::vector<std::string> params;  // 파라미터 바인딩용 (필요시 확장)
    std::function<void(bool, sql::ResultSet*)> callback;
};

class DBManager
{
public:
    DBManager();
    ~DBManager();

    void Initialize(const std::string& host, const std::string& user, const std::string& password, const std::string& schema);

    // 비동기 쿼리 요청 (SELECT)
    void RequestQuery(const std::string& sql, std::function<void(bool, sql::ResultSet*)> callback);

    // 비동기 실행 요청 (INSERT/UPDATE/DELETE) - 결과값 없는 작업
    void RequestExecute(const std::string& sql);

    // 캐릭터 레벨/경험치 비동기 업데이트 (비동기)
    void UpdateCharacterLevelAndExpAsync(unsigned int characterID, unsigned int level, unsigned long exp);

    // 모든 캐릭터 조회 (동기 - 동시에 여러 쓰레드가 호출하면 문제)
    std::vector<CharacterData> GetAllCharacters();

private:
    void DBThreadLoop();
    void Shutdown();

    std::queue<DBRequest> m_requestQueue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::thread m_thread;

    sql::mysql::MySQL_Driver* m_driver = nullptr;
    std::unique_ptr<sql::Connection> m_con;
    std::mutex m_conMutex;  // DB 연결 동기화용

    std::atomic<bool> m_running{ true };
};
