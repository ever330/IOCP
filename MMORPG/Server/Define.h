#pragma once

#define BUFFER_SIZE 1024
#define MAX_USER_COUNT 1000
#define PORT 3030

// MainServer에서 서버 콘텐츠 용으로 사용할 스레드 수
#define CONTENTS_THREAD 2

#define HEARTBEAT_INTERVAL_MS 5000
#define TIMEOUT_MS 15000
#define SPAWN_COUNT 50

#define READ 10
#define WRITE 11
#define ACCEPT 12
#define LOCAL_ADDR_SIZE  (sizeof(sockaddr_in) + 16)
#define REMOTE_ADDR_SIZE (sizeof(sockaddr_in) + 16)

// 패킷용
#define NAME_SIZE 10
#define MSG_SIZE 256