#pragma once

#include "pch.h"
#include "Util.h"
#include "Define.h"
#include "packet.h"
#include "OtherPlayer.h"
#include "Monster.h"

class Network;

class CTestView : public CView
{
protected:
    CPoint m_playerPos;
	Network* m_network;
    CString m_userNickname;
	Direction m_playerDirection;
    std::unordered_map<uint16_t, std::unique_ptr<Monster>> m_monsters;
    std::unordered_map<uint16_t, OtherPlayer> m_otherPlayers;

    // ������Ʈ��
    DWORD m_prevTime = 0;

    // ���� ���� ��� ����
    DWORD m_lastAttackTime = 0;
    bool m_isAttacking = false;
    DWORD m_attackStartTime = 0;
    const DWORD m_attackDuration = 200; // ���� ��� ���� �ð� (��: 200ms)
    CPoint m_attackStartPos;       // ���� ���� �� �÷��̾� ��ġ
    Direction m_attackDirection;   // ���� ����

	std::unordered_map<uint16_t, AttackInfo> m_otherAttacks; // �ٸ� �÷��̾��� ���� ����

public:
    CTestView();
    virtual ~CTestView();

    virtual void OnDraw(CDC* pDC);
    virtual void OnInitialUpdate();
    virtual BOOL OnEraseBkgnd(CDC* pDC);

    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    void SetNetwork(Network* pNetwork) { m_network = pNetwork; }
	void SetUserNickname(const CString& nickname) { m_userNickname = nickname; }

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg UINT OnGetDlgCode();
    afx_msg LRESULT OnUpdateMonsterState(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMapChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlayerEnter(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlayerLeave(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdatePlayerState(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnUpdateMonsterHit(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnRespawnMonster(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlayerAttack(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
    void DrawScene(CDC* pDC);
	void DrawAttackEffect(CDC* pDC, CPoint startPos, Direction AttackDir);
};