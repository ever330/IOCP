#pragma once

#define BUFFER_SIZE 1024
#define MAX_USER_COUNT 1000
#define PORT 3030

// 각 용도별 스레드 카운트
#define IOCP_THREAD 8
#define PACKET_THREAD 8

#define HEARTBEAT_INTERVAL_MS 5000
#define TIMEOUT_MS 15000
#define SPAWN_COUNT 50

#define READ 10
#define WRITE 11
#define ACCEPT 12
#define LOCAL_ADDR_SIZE  (sizeof(sockaddr_in) + 16)
#define REMOTE_ADDR_SIZE (sizeof(sockaddr_in) + 16)

// 패킷용
#define ID_SIZE 20
#define PASSWORD_SIZE 20
#define NAME_SIZE 10
#define MSG_SIZE 256

#define MAP_MAX_X 490
#define MAP_MAX_Y 330