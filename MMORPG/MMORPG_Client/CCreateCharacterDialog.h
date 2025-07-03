#pragma once

#include "pch.h"
#include "afxdialogex.h"
#include "User.h"
#include "Network.h"
#include "resource.h"

class CCreateCharacterDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CCreateCharacterDialog)

public:
    CCreateCharacterDialog(CWnd* pParent = nullptr);
    virtual ~CCreateCharacterDialog();

    // ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_CREATE_CHARACTER_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

    virtual BOOL OnInitDialog();

public:
    void SetUser(User* user);
    void SetNetwork(Network* network);

private:
    User* m_user;
    Network* m_network;
	int m_gender; // 0: ����, 1: ����

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedNameCheckbutton();
    afx_msg void OnBnClickedCreateButton();
    afx_msg LRESULT OnCreateSuccess(WPARAM wParam, LPARAM lParam);
};