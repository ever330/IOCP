#pragma once
#include "afxdialogex.h"
#include "Network.h"

class CSignUpDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSignUpDialog)

public:
	CSignUpDialog(CWnd* pParent = nullptr);   // ǥ�� �������Դϴ�.
	virtual ~CSignUpDialog();

	// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIGNUP_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBtnCheckId();
	afx_msg void OnBnClickedBtnSignUp();

	void SetNetwork(Network* network) { m_network = network; }

private:
	Network* m_network;
public:
	afx_msg void OnClickedIdCheck();
	afx_msg void OnClickedSignupSignup();
	afx_msg void OnBnClickedSignupCancle();
};