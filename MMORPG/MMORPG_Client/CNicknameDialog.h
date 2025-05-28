#pragma once
#include "afxdialogex.h"


// CNicknameDialog 대화 상자

class CNicknameDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CNicknameDialog)

public:
	CNicknameDialog(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CNicknameDialog();
	CString m_strNickname;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NICKNAME_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
};
