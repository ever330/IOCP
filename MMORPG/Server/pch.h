#ifndef PCH_H
#define PCH_H

#define _WINSOCKAPI_

#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <map>
#include <sstream>
#include <ctime>
#include <Windows.h>
#include <time.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <concurrent_queue.h>

// Socket
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <Ws2tcpip.h>

// 멀티스레드 관련
#include <thread>
#include <mutex>

// DB
#include <mysql/jdbc.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")
#pragma comment(lib, "Mswsock.lib")

#endif //PCH_H