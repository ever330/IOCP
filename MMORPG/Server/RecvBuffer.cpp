#include "pch.h"
#include "RecvBuffer.h"
#include "Packet.h"

RecvBuffer::RecvBuffer(int bufferSize)
    : m_bufferSize(bufferSize)
    , m_readPos(0)
    , m_writePos(0)
{
    m_buffer = new char[bufferSize];
}

RecvBuffer::~RecvBuffer()
{
    delete[] m_buffer;
}

bool RecvBuffer::Write(const char* data, int len)
{
    if (len > GetFreeSize())
    {
        // 남은 공간보다 크면 Compact 시도
        Compact();
        if (len > GetFreeSize())
            return false;  // 그래도 부족하면 실패
    }

    memcpy(m_buffer + m_writePos, data, len);
    m_writePos += len;
    return true;
}

bool RecvBuffer::HasCompletePacket() const
{
    int dataSize = GetDataSize();

    // 헤더 크기보다 작으면 패킷 없음
    if (dataSize < sizeof(PacketBase))
        return false;

    // 헤더에서 패킷 크기 읽기
    const PacketBase* header = reinterpret_cast<const PacketBase*>(m_buffer + m_readPos);

    // 패킷 크기 유효성 검사
    if (header->PacketSize < sizeof(PacketBase) || header->PacketSize > RECV_BUFFER_SIZE)
        return false;

    // 완전한 패킷이 왔는지 확인
    return dataSize >= header->PacketSize;
}

bool RecvBuffer::ReadPacket(char* outBuffer, int& outLen)
{
    if (!HasCompletePacket())
        return false;

    const PacketBase* header = reinterpret_cast<const PacketBase*>(m_buffer + m_readPos);
    int packetSize = header->PacketSize;

    // 패킷 복사
    memcpy(outBuffer, m_buffer + m_readPos, packetSize);
    outLen = packetSize;

    // 읽기 위치 이동
    m_readPos += packetSize;

    // 버퍼를 비었으면 위치 초기화
    if (m_readPos == m_writePos)
    {
        m_readPos = 0;
        m_writePos = 0;
    }
    // 읽기 위치가 너무 뒤에 있으면 Compact
    else if (m_readPos > m_bufferSize / 2)
    {
        Compact();
    }

    return true;
}

int RecvBuffer::GetDataSize() const
{
    return m_writePos - m_readPos;
}

int RecvBuffer::GetFreeSize() const
{
    return m_bufferSize - m_writePos;
}

void RecvBuffer::Compact()
{
    int dataSize = GetDataSize();
    if (dataSize > 0 && m_readPos > 0)
    {
        memmove(m_buffer, m_buffer + m_readPos, dataSize);
    }
    m_readPos = 0;
    m_writePos = dataSize;
}

void RecvBuffer::Clear()
{
    m_readPos = 0;
    m_writePos = 0;
}

char* RecvBuffer::GetWritePtr()
{
    return m_buffer + m_writePos;
}

void RecvBuffer::OnWrite(int len)
{
    m_writePos += len;
}
