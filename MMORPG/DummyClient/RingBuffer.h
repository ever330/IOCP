#pragma once

#include <iostream>
#include <vector>
#include <mutex>

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
        std::lock_guard<std::mutex> lock(mutex);

        if (FreeSpace() < size)
            return false;  // 공간 부족하면 실패

        for (size_t i = 0; i < size; i++)
        {
            buffer[tail] = data[i];
            tail = (tail + 1) % capacity;
        }

        if (tail == head)
            isFull = true;

        return true;
    }

    // 데이터 읽기: 데이터가 충분할 때만 읽기
    bool Read(char* outData, size_t size)
    {
        std::lock_guard<std::mutex> lock(mutex);

        if (DataSize() < size)
            return false;  // 데이터 부족하면 실패

        for (size_t i = 0; i < size; i++)
        {
            outData[i] = buffer[head];
            head = (head + 1) % capacity;
        }

        isFull = false;

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
	std::mutex mutex;
};