#pragma once

#include "afxdialogex.h"
#include "Network.h"

class CLoginDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDialog)

public:
	CLoginDialog(CWnd* pParent = nullptr);   // ǥ�� �������Դϴ�.
	virtual ~CLoginDialog();

	void SetNetwork(Network* network) { m_network = network; }

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()

private:
	Network* m_network;
public:
	afx_msg void OnClickedLoginSignup();
	afx_msg void OnClickedLogin();
};