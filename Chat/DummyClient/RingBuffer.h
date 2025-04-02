#pragma once

#include <iostream>
#include <vector>
#include <mutex>

class RingBuffer
{
public:
	RingBuffer(size_t size) : buffer(size), capacity(size), head(0), tail(0), isFull(false) {}

	// �̵� ������ �߰�
	RingBuffer(RingBuffer&& other) noexcept
		: buffer(std::move(other.buffer)), capacity(other.capacity),
		head(other.head), tail(other.tail), isFull(other.isFull) {}

	// �̵� �Ҵ� ������ �߰�
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

	// ���� �����ڿ� ���� �Ҵ� �����ڴ� ����
	RingBuffer(const RingBuffer&) = delete;
	RingBuffer& operator=(const RingBuffer&) = delete;

	// ������ ���� (�����ϸ� true, ���۰� ���� �� ������ false)
	bool Write(const char* data, size_t size) 
	{
		std::lock_guard<std::mutex> lock(mutex);

		if (IsFull())
			return false;

		for (size_t i = 0; i < size; i++) 
		{
			buffer[tail] = data[i];
			tail = (tail + 1) % capacity;
			if (tail == head) isFull = true;  // ���۰� ���� á�� ��� ǥ��
		}
		return true;
	}

	// ������ �б� (�����ϸ� true, ���۰� ��� ������ false)
	bool Read(char* outData, size_t size) 
	{
		std::lock_guard<std::mutex> lock(mutex);

		if (IsEmpty())
			return false;

		for (size_t i = 0; i < size; i++) 
		{
			if (IsEmpty()) return false;  // �߰��� �����Ͱ� ������ ��� ����
			outData[i] = buffer[head];
			head = (head + 1) % capacity;
			isFull = false;
		}
		return true;
	}

	// ���۰� ��� �ִ��� Ȯ��
	bool IsEmpty() 
	{
		return (head == tail) && !isFull;
	}

	// ���۰� ���� á���� Ȯ��
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