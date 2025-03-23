#pragma once

#include <winsock2.h>

#include "pch.h"
#include "Define.h"

struct Session
{
    OVERLAPPED overlapped;
    WSABUF wsabuf;
    char buffer[BUFFER_SIZE];
    SOCKET socket;
};