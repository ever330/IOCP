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
		head(other.head), tail(other.tail), isFull(other.isFull) {}

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

	// 데이터 쓰기 (성공하면 true, 버퍼가 가득 차 있으면 false)
	bool Write(const char* data, size_t size) 
	{
		std::lock_guard<std::mutex> lock(mutex);

		if (IsFull())
			return false;

		for (size_t i = 0; i < size; i++) 
		{
			buffer[tail] = data[i];
			tail = (tail + 1) % capacity;
			if (tail == head) isFull = true;  // 버퍼가 가득 찼을 경우 표시
		}
		return true;
	}

	// 데이터 읽기 (성공하면 true, 버퍼가 비어 있으면 false)
	bool Read(char* outData, size_t size) 
	{
		std::lock_guard<std::mutex> lock(mutex);

		if (IsEmpty())
			return false;

		for (size_t i = 0; i < size; i++) 
		{
			if (IsEmpty()) return false;  // 중간에 데이터가 부족할 경우 종료
			outData[i] = buffer[head];
			head = (head + 1) % capacity;
			isFull = false;
		}
		return true;
	}

	// 버퍼가 비어 있는지 확인
	bool IsEmpty() 
	{
		return (head == tail) && !isFull;
	}

	// 버퍼가 가득 찼는지 확인
	bool IsFull() 
	{
		return isFull;
	}

private:
	std::vector<char> buffer;
	size_t capacity;
	size_t head;
	size_t tail;
	bool isFull;
	std::mutex mutex;
};