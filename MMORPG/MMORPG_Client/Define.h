#pragma once

// 패킷용
#define ID_SIZE 20
#define PASSWORD_SIZE 20
#define NAME_SIZE 10
#define MSG_SIZE 256

// 맵 크기
#define MAP_MAX_X 490
#define MAP_MAX_Y 330

// 위치 보정용
#define THRESHOLD 30

#define WM_UPDATE_MONSTER_STATE (WM_USER + 100)
#define WM_UPDATE_PLAYER_STATE (WM_USER + 101)
#define WM_OTHER_PLAYER_POS_SYNC (WM_USER + 102)
#define WM_UPDATE_MONSTER_HIT (WM_USER + 103)
#define WM_MONSTER_RESPAWN (WM_USER + 104)
#define WM_PLAYER_ATTACK (WM_USER + 105)
#define WM_PLAYER_POS_SYNC (WM_USER + 106)
#define WM_EXP_GAIN (WM_USER + 107)

#define WM_PLAYER_ENTER   (WM_USER + 2)
#define WM_PLAYER_LEAVE   (WM_USER + 3)

#define WM_MAP_CHANGE   (WM_USER + 10)

#define WM_SIGNUP_SUCCESS (WM_USER + 200)
#define WM_LOGIN_SUCCESS (WM_USER + 201)
#define WM_CREATE_SUCCESS (WM_USER + 201)
#define WM_CHARACTER_SELECT_SUCCESS (WM_USER + 202)

#define WM_SHOW_RANKING (WM_USER + 301)