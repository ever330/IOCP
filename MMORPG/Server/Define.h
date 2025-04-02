#pragma once

#define BUFFER_SIZE 1024
#define MAX_USER_COUNT 1000
#define PORT 3030

#define READ 10
#define WRITE 11
#define ACCEPT 12
#define LOCAL_ADDR_SIZE  (sizeof(sockaddr_in) + 16)
#define REMOTE_ADDR_SIZE (sizeof(sockaddr_in) + 16)

// ÆÐÅ¶¿ë
#define NAME_SIZE 40
#define MSG_SIZE 256