#pragma once

// ��Ŷ��
#define NAME_SIZE 10
#define MSG_SIZE 256

// �� ũ��
#define MAP_MAX_X 490
#define MAP_MAX_Y 330

// CView��
#define WM_UPDATE_MONSTER_STATE (WM_USER + 100)
#define WM_UPDATE_PLAYER_STATE (WM_USER + 101)
#define WM_OTHER_PLAYER_MOVE (WM_USER + 102)
#define WM_UPDATE_MONSTER_HIT (WM_USER + 103)
#define WM_MONSTER_RESPAWN (WM_USER + 104)
#define WM_PLAYER_ATTACK (WM_USER + 105)

#define WM_PLAYER_ENTER   (WM_USER + 2)
#define WM_PLAYER_LEAVE   (WM_USER + 3)

#define WM_MAP_CHANGE   (WM_USER + 10)