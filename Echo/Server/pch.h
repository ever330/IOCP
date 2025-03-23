#ifndef PCH_H
#define PCH_H

#define _WINSOCKAPI_

#include <iostream>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <sstream>
#include <ctime>
#include <Windows.h>
#include <time.h>

// Socket
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>

// 멀티스레드 관련
#include <thread>
#include <mutex>

// DB
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "odbc32.lib")

#endif //PCH_H