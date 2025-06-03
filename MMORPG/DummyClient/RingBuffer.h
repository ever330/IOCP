#pragma once

#include "pch.h"

class RingBuffer
{
public:
    RingBuffer(size_t size) : buffer(size), capacity(size), head(0), tail(0), isFull(false) {}

    // �̵� ������ �߰�
    RingBuffer(RingBuffer&& other) noexcept
        : buffer(std::move(other.buffer)), capacity(other.capacity),
        head(other.head), tail(other.tail), isFull(other.isFull) {
    }

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

    // ������ ����: ������ ����� ���� ����
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

    // ������ �б�: �����Ͱ� ����� ���� �б�
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

    // ������ �̸����� (���� �ʰ� ���縸)
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

    // ���۰� ����ִ��� Ȯ��
    bool IsEmpty()
    {
        return (head == tail) && !isFull;
    }

    // ���۰� ���� á���� Ȯ��
    bool IsFull()
    {
        return isFull;
    }

    // ��� ������ ���� ũ�� ��ȯ
    size_t FreeSpace()
    {
        if (isFull)
            return 0;
        if (tail >= head)
            return capacity - (tail - head);
        else
            return head - tail;
    }

    // ��� ���� ������ ũ�� ��ȯ
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