
// MMORPG_ClientDlg.h: 헤더 파일
//

#pragma once

#include "Network.h"
#include "CChatInputEdit.h"
#include "CTestView.h"

// CMMORPGClientDlg 대화 상자
class CMMORPGClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CMMORPGClientDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MMORPG_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	Network m_network;

	CString m_strUserNickname;

	CRichEditCtrl m_chatOutput; // 채팅 내용 출력 창
	CChatInputEdit m_chatInput; // 입력창
	CButton m_sendButton;       // 전송 버튼

	CComboBox m_mapList;
	CButton m_moveButton;

	CStatic m_mapNameStatic;
	CStatic m_userNicknameStatic;

	CTestView* m_testView = nullptr; // 맵 뷰

	void AddChatMessage(const CString& message);
	afx_msg void OnSendButtonClicked(); // 채팅 전송 버튼
	afx_msg void OnMoveButtonClicked(); // 맵 이동 버튼

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
