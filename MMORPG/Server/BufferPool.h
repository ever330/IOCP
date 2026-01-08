#pragma once

#include "pch.h"
#include "Define.h"

// 패킷 크기별 버퍼 크기 정의
constexpr int SMALL_BUFFER_SIZE = 256;
constexpr int MEDIUM_BUFFER_SIZE = 1024;
constexpr int LARGE_BUFFER_SIZE = 4096;

// 초기 풀 크기
constexpr int INITIAL_POOL_SIZE = 100;

template<int BufferSize>
class BufferPool
{
public:
    static BufferPool& Instance()
    {
        static BufferPool instance;
        return instance;
    }

    void Initialize(int initialCount = INITIAL_POOL_SIZE)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (int i = 0; i < initialCount; ++i)
        {
            m_freeBuffers.push(new char[BufferSize]);
        }
        m_totalAllocated = initialCount;
    }

    char* Acquire()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_freeBuffers.empty())
        {
            // 풀이 비었으면 새로 할당
            m_totalAllocated++;
            return new char[BufferSize];
        }

        char* buffer = m_freeBuffers.front();
        m_freeBuffers.pop();
        return buffer;
    }

    void Release(char* buffer)
    {
        if (buffer == nullptr)
            return;

        std::lock_guard<std::mutex> lock(m_mutex);

        // 너무 많이 쌓이면 메모리 해제
        if (m_freeBuffers.size() > m_totalAllocated * 2)
        {
            delete[] buffer;
            m_totalAllocated--;
        }
        else
        {
            m_freeBuffers.push(buffer);
        }
    }

    void Shutdown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_freeBuffers.empty())
        {
            delete[] m_freeBuffers.front();
            m_freeBuffers.pop();
        }
        m_totalAllocated = 0;
    }

    size_t GetFreeCount() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_freeBuffers.size();
    }

    size_t GetTotalAllocated() const
    {
        return m_totalAllocated;
    }

private:
    BufferPool() : m_totalAllocated(0) {}
    ~BufferPool() { Shutdown(); }

    BufferPool(const BufferPool&) = delete;
    BufferPool& operator=(const BufferPool&) = delete;

    std::queue<char*> m_freeBuffers;
    mutable std::mutex m_mutex;
    std::atomic<size_t> m_totalAllocated;
};

// 타입 별칭
using SmallBufferPool = BufferPool<SMALL_BUFFER_SIZE>;
using MediumBufferPool = BufferPool<MEDIUM_BUFFER_SIZE>;
using LargeBufferPool = BufferPool<LARGE_BUFFER_SIZE>;

// 버퍼 크기에 따라 적절한 버퍼 획득
inline char* AcquireBuffer(int requiredSize)
{
    if (requiredSize <= SMALL_BUFFER_SIZE)
        return SmallBufferPool::Instance().Acquire();
    else if (requiredSize <= MEDIUM_BUFFER_SIZE)
        return MediumBufferPool::Instance().Acquire();
    else if (requiredSize <= LARGE_BUFFER_SIZE)
        return LargeBufferPool::Instance().Acquire();
    else
        return new char[requiredSize];  // 대형 버퍼는 직접 할당
}

// 버퍼 반환 (크기 기반으로 어떤 풀에 반환할지 결정)
inline void ReleaseBuffer(char* buffer, int bufferSize)
{
    if (buffer == nullptr)
        return;

    if (bufferSize <= SMALL_BUFFER_SIZE)
        SmallBufferPool::Instance().Release(buffer);
    else if (bufferSize <= MEDIUM_BUFFER_SIZE)
        MediumBufferPool::Instance().Release(buffer);
    else if (bufferSize <= LARGE_BUFFER_SIZE)
        LargeBufferPool::Instance().Release(buffer);
    else
        delete[] buffer;
}

// 모든 버퍼풀 초기화
inline void InitializeBufferPools()
{
    SmallBufferPool::Instance().Initialize(200);   // 소형 패킷용 (로그인 등)
    MediumBufferPool::Instance().Initialize(100);  // 중형 패킷용
    LargeBufferPool::Instance().Initialize(50);    // 대형 패킷용
}

// 모든 버퍼풀 정리
inline void ShutdownBufferPools()
{
    SmallBufferPool::Instance().Shutdown();
    MediumBufferPool::Instance().Shutdown();
    LargeBufferPool::Instance().Shutdown();
}

// RAII 래퍼 - 자동으로 버퍼 반환
class PooledBuffer
{
public:
    PooledBuffer(int size) : m_size(size)
    {
        m_buffer = AcquireBuffer(size);
    }

    ~PooledBuffer()
    {
        if (m_buffer)
            ReleaseBuffer(m_buffer, m_size);
    }

    // 이동 생성자
    PooledBuffer(PooledBuffer&& other) noexcept
        : m_buffer(other.m_buffer), m_size(other.m_size)
    {
        other.m_buffer = nullptr;
        other.m_size = 0;
    }

    // 이동 대입 연산자
    PooledBuffer& operator=(PooledBuffer&& other) noexcept
    {
        if (this != &other)
        {
            if (m_buffer)
                ReleaseBuffer(m_buffer, m_size);
            m_buffer = other.m_buffer;
            m_size = other.m_size;
            other.m_buffer = nullptr;
            other.m_size = 0;
        }
        return *this;
    }

    // 복사 금지
    PooledBuffer(const PooledBuffer&) = delete;
    PooledBuffer& operator=(const PooledBuffer&) = delete;

    char* Get() { return m_buffer; }
    const char* Get() const { return m_buffer; }
    int Size() const { return m_size; }

    // 소유권 해제 (버퍼 반환 안함)
    char* Release()
    {
        char* buf = m_buffer;
        m_buffer = nullptr;
        m_size = 0;
        return buf;
    }

private:
    char* m_buffer;
    int m_size;
};
