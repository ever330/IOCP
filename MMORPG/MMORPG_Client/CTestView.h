#pragma once

#include "pch.h"
#include "Util.h"
#include "Define.h"
#include "packet.h"
#include "OtherPlayer.h"
#include "Monster.h"
#include "User.h"

class Network;

class CTestView : public CView
{
public:
    CTestView();
    virtual ~CTestView();

    virtual void OnDraw(CDC* pDC);
    virtual void OnInitialUpdate();
    virtual BOOL OnEraseBkgnd(CDC* pDC);

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    void SetNetwork(Network* network) { m_network = network; }
    void SetUser(User* user) { m_user = user; }

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg UINT OnGetDlgCode();
    afx_msg LRESULT OnUpdateMonsterState(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMapChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlayerEnter(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlayerLeave(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdatePlayerState(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnOtherPlayerMove(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlayerPosSync(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdateMonsterHit(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRespawnMonster(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlayerAttack(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
    void DrawScene(CDC* pDC);
	void DrawAttackEffect(CDC* pDC, CPoint startPos, Direction AttackDir);
    void UpdatePlayer(float deltaTime);
	void UpdateOtherPlayers(float deltaTime);
	void ClampPosition(CPoint& pos);

    CPoint m_playerPos;
    Network* m_network;
    User* m_user;
    Direction m_playerDirection;
    std::unordered_map<uint16_t, std::unique_ptr<Monster>> m_monsters;
    std::unordered_map<uint16_t, OtherPlayer> m_otherPlayers;

    // ���� ���� ��� ����
    DWORD m_lastAttackTime;
    bool m_isAttacking;
    DWORD m_attackStartTime;
    const DWORD m_attackDuration = 200; // ���� ��� ���� �ð� (��: 200ms)
    CPoint m_attackStartPos;       // ���� ���� �� �÷��̾� ��ġ
    Direction m_attackDirection;   // ���� ����
    bool m_isMoving; // �÷��̾ �̵� ������ ����
    std::chrono::steady_clock::time_point m_lastUpdateTime; // ������ ������Ʈ �ð�

    std::unordered_map<uint16_t, AttackInfo> m_otherAttacks; // �ٸ� �÷��̾��� ���� ����

    std::deque<PlayerInput> m_inputBuffer; // �Է� ����
    uint32_t m_currentFrameID;
};