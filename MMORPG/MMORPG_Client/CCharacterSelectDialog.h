#pragma once

#include "pch.h"
#include "afxdialogex.h"
#include "User.h"
#include "Network.h"
#include "resource.h"
#include "CCreateCharacterDialog.h"

class CCharacterSelectDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CCharacterSelectDialog)

public:
    CCharacterSelectDialog(CWnd* pParent = nullptr);
    virtual ~CCharacterSelectDialog();

    // 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_SELECT_CHARACTER_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

    virtual BOOL OnInitDialog();

public:
    void SetUser(User* user);
    void SetNetwork(Network* network);

private:
    void RefreshCharacterList();

    CListCtrl m_characterList;
    CStatic m_characterInfo;
    std::vector<uint16_t> m_characterIndexMap;

	User* m_user;
	Network* m_network;

public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedGamestart();
    afx_msg void OnBnClickedCreateCharacter();
    afx_msg void OnLvnItemchangedCharacterList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnCharacterSelectAck(WPARAM wParam, LPARAM lParam);
};