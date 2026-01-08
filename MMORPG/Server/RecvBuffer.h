#pragma once

#include "pch.h"
#include "Define.h"

constexpr int RECV_BUFFER_SIZE = 8192;

class RecvBuffer
{
public:
    RecvBuffer(int bufferSize = RECV_BUFFER_SIZE);
    ~RecvBuffer();

    // 복사 금지
    RecvBuffer(const RecvBuffer&) = delete;
    RecvBuffer& operator=(const RecvBuffer&) = delete;

    // 버퍼에 데이터 쓰기 (recv 후 호출)
    bool Write(const char* data, int len);

    // 완전한 패킷이 있는지 확인
    bool HasCompletePacket() const;

    // 완전한 패킷 읽기 (성공 시 true, 패킷 데이터와 크기 반환)
    bool ReadPacket(char* outBuffer, int& outLen);

    // 현재 저장되어 있는 데이터 크기
    int GetDataSize() const;

    // 남아 있는 공간 크기
    int GetFreeSize() const;

    // 버퍼 정리 (데이터를 앞으로 이동)
    void Compact();

    // 버퍼 초기화
    void Clear();

    // 쓰기 위치 포인터 반환 (WSARecv용)
    char* GetWritePtr();

    // 쓰기 완료 후 writePos 이동
    void OnWrite(int len);

private:
    char* m_buffer;
    int m_bufferSize;
    int m_readPos;
    int m_writePos;
};
