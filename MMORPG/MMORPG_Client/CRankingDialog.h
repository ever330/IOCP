#pragma once

#include "pch.h"
#include "afxdialogex.h"
#include "Network.h"
#include "resource.h"

class CRankingDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CRankingDialog)

public:
    CRankingDialog(CWnd* pParent = nullptr);
    virtual ~CRankingDialog();

    // 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_RANKING_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    virtual void PostNcDestroy();
    DECLARE_MESSAGE_MAP()

public:
    void SetRankingData(const uint16_t myRanking, const std::vector<S2CRankingInfo>& rankings);

private:
	void UpdateRankingList(const uint16_t myRanking, const std::vector<S2CRankingInfo>&);
    void RefreshCharacterList();

	CListCtrl m_rankingListCtrl;
	CStatic m_myRankingStatic;

	uint16_t m_myRanking = 0;
	std::vector<S2CRankingInfo> m_rankings;
public:
    afx_msg void OnBnClickedRankingCloseButton();
};