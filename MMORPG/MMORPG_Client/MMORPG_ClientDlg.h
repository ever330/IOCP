
// MMORPG_ClientDlg.h: 헤더 파일
//

#pragma once

#include "Network.h"
#include "CChatInputEdit.h"
#include "CTestView.h"
#include "User.h"

class CRankingDialog;

// CMMORPGClientDlg 대화 상자
class CMMORPGClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CMMORPGClientDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

	void SetNetwork(Network* network) { m_network = network; }
	void SetUser(User* user) { m_user = user; }
	void ShowRankingDialog(uint16_t myRank, const std::vector<S2CRankingInfo>& rankings);
	void OnRankingDialogClosed();
	void AddChatMessage(const CString& message);

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MMORPG_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	Network* m_network;
	User* m_user;
	CRankingDialog* m_rankingDialog = nullptr; // 랭킹 대화 상자

	CRichEditCtrl m_chatOutput; // 채팅 내용 출력 창
	CChatInputEdit m_chatInput; // 입력창
	CButton m_sendButton;       // 전송 버튼

	CComboBox m_mapList;
	CButton m_moveButton;

	CStatic m_mapNameStatic;
	CStatic m_nicknameStatic;
	CStatic m_characterInfoStatic;

	CTestView* m_testView = nullptr; // 맵 뷰

	bool m_rankingRequested = false;

	afx_msg void OnSendButtonClicked(); // 채팅 전송 버튼
	afx_msg void OnMoveButtonClicked(); // 맵 이동 버튼
	afx_msg void OnBnClickedRankingButton();
	afx_msg LRESULT OnShowRanking(WPARAM wParam, LPARAM lParam);

	void SetChatIO();
	void SetMapList();
	void SetGameView();
	void SetCallback();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnDestroy();
	virtual void OnClose();
	virtual void OnCancel();

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
