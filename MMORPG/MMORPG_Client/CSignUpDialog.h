#pragma once
#include "afxdialogex.h"
#include "Network.h"

class CSignUpDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CSignUpDialog)

public:
	CSignUpDialog(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CSignUpDialog();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SIGNUP_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	HICON m_hIcon;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	void SetNetwork(Network* network) { m_network = network; }

private:
	Network* m_network;
	CEdit m_idEdit;
	CEdit m_passwordEdit;
	CEdit m_passwordConfirmEdit;

public:
	afx_msg void OnClickedIdCheck();
	afx_msg void OnClickedSignupSignup();
	afx_msg void OnEnChangeIDEdit();
	afx_msg void OnEnChangePasswordEdit();
	afx_msg void OnEnChangePasswordConfirmEdit();
	afx_msg LRESULT OnSignUpSuccess(WPARAM wParam, LPARAM lParam);
};