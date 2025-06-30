#pragma once

#include "afxdialogex.h"
#include "Network.h"
#include "User.h"

class CLoginDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDialog)

public:
	CLoginDialog(CWnd* pParent = nullptr);   // ǥ�� �������Դϴ�.
	virtual ~CLoginDialog();

	void SetNetwork(Network* network);
	void SetUser(User* user);

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	Network* m_network;
	User* m_user;
	CEdit m_idEdit;
	CEdit m_passwordEdit;

public:
	afx_msg void OnClickedLoginSignup();
	afx_msg void OnClickedLogin();
	afx_msg LRESULT OnLoginSuccess(WPARAM wParam, LPARAM lParam);
};