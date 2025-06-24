#pragma once

#include "afxdialogex.h"
#include "Network.h"

class CLoginDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDialog)

public:
	CLoginDialog(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CLoginDialog();

	void SetNetwork(Network* network) { m_network = network; }

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

private:
	Network* m_network;
public:
	afx_msg void OnClickedLoginSignup();
	afx_msg void OnClickedLogin();
};