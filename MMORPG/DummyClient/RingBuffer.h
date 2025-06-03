#pragma once

#include "pch.h"

class RingBuffer
{
public:
    RingBuffer(size_t size) : buffer(size), capacity(size), head(0), tail(0), isFull(false) {}

    // 이동 생성자 추가
    RingBuffer(RingBuffer&& other) noexcept
        : buffer(std::move(other.buffer)), capacity(other.capacity),
        head(other.head), tail(other.tail), isFull(other.isFull) {
    }

    // 이동 할당 연산자 추가
    RingBuffer& operator=(RingBuffer&& other) noexcept
    {
        if (this != &other) {
            buffer = std::move(other.buffer);
            capacity = other.capacity;
            head = other.head;
            tail = other.tail;
            isFull = other.isFull;
        }
        return *this;
    }

    // 복사 생성자와 복사 할당 연산자는 삭제
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    // 데이터 쓰기: 공간이 충분할 때만 쓰기
    bool Write(const char* data, size_t size)
    {
        if (FreeSpace() < size)
            return false;

        size_t firstPart = min(size, capacity - tail);
        std::memcpy(&buffer[tail], data, firstPart);
        std::memcpy(&buffer[0], data + firstPart, size - firstPart);
        tail = (tail + size) % capacity;

        if (tail == head)
            isFull = true;

        return true;
    }

    // 데이터 읽기: 데이터가 충분할 때만 읽기
    bool Read(char* outData, size_t size)
    {
        if (DataSize() < size)
            return false;

        size_t firstPart = min(size, capacity - head);
        std::memcpy(outData, &buffer[head], firstPart);
        std::memcpy(outData + firstPart, &buffer[0], size - firstPart);
        head = (head + size) % capacity;

        isFull = false;
        return true;
    }

    // 데이터 미리보기 (읽지 않고 복사만)
    bool Peek(char* outData, size_t size)
    {
        if (DataSize() < size)
            return false;

        size_t tempHead = head;
        size_t firstPart = min(size, capacity - tempHead);
        std::memcpy(outData, &buffer[tempHead], firstPart);
        std::memcpy(outData + firstPart, &buffer[0], size - firstPart);

        return true;
    }

    // 버퍼가 비어있는지 확인
    bool IsEmpty()
    {
        return (head == tail) && !isFull;
    }

    // 버퍼가 가득 찼는지 확인
    bool IsFull()
    {
        return isFull;
    }

    // 사용 가능한 공간 크기 반환
    size_t FreeSpace()
    {
        if (isFull)
            return 0;
        if (tail >= head)
            return capacity - (tail - head);
        else
            return head - tail;
    }

    // 사용 중인 데이터 크기 반환
    size_t DataSize()
    {
        if (isFull)
            return capacity;
        if (tail >= head)
            return tail - head;
        else
            return capacity - (head - tail);
    }

private:
    std::vector<char> buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    bool isFull;
};