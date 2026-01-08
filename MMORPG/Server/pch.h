#ifndef PCH_H
#define PCH_H

// Winsock2를 Windows.h 보다 먼저 include 해야 함
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <mswsock.h>
#include <Ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <map>
#include <sstream>
#include <ctime>
#include <time.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <concurrent_queue.h>
#include <optional>
#include <atomic>

// 메모리 누수 검사
#include <vld.h>

// 멀티쓰레딩 관련
#include <thread>
#include <mutex>

// DB
#include <mysql/jdbc.h>

// Redis
#include <hiredis/hiredis.h>

// 암호화용
#include <openssl/sha.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "Mswsock.lib")

#endif //PCH_H
