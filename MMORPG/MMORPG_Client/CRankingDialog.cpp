#include "CRankingDialog.h"
#include "MMORPG_ClientDlg.h"

IMPLEMENT_DYNAMIC(CRankingDialog, CDialogEx)

CRankingDialog::CRankingDialog(CWnd* pParent)
    : CDialogEx(IDD_RANKING_DIALOG, pParent)
{
}

CRankingDialog::~CRankingDialog()
{
}

BOOL CRankingDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_rankingListCtrl.ModifyStyle(0, LVS_REPORT);
    m_rankingListCtrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    m_rankingListCtrl.InsertColumn(0, _T("����"), LVCFMT_CENTER, 50);
    m_rankingListCtrl.InsertColumn(1, _T("�̸�"), LVCFMT_LEFT, 100);
    m_rankingListCtrl.InsertColumn(2, _T("����"), LVCFMT_CENTER, 50);

	UpdateRankingList(m_myRanking, m_rankings);

    return TRUE;
}

void CRankingDialog::PostNcDestroy()
{
    CDialogEx::PostNcDestroy();

    // ���� ���̾�α׿��� �Ҹ� �˸�
    if (AfxGetMainWnd())
        ((CMMORPGClientDlg*)AfxGetMainWnd())->OnRankingDialogClosed();
}

BEGIN_MESSAGE_MAP(CRankingDialog, CDialogEx)
    ON_BN_CLICKED(IDC_RANKING_CLOSE_BUTTON, &CRankingDialog::OnBnClickedRankingCloseButton)
END_MESSAGE_MAP()

void CRankingDialog::SetRankingData(const uint16_t myRanking, const std::vector<S2CRankingInfo>& rankings)
{
	m_myRanking = myRanking;
	m_rankings = rankings;
}

void CRankingDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_RANKING_LIST, m_rankingListCtrl);
	DDX_Control(pDX, IDC_MY_RANKING_STATIC, m_myRankingStatic);
}

void CRankingDialog::UpdateRankingList(const uint16_t myRanking, const std::vector<S2CRankingInfo>& rankings)
{
    m_rankingListCtrl.DeleteAllItems();  // ���� �׸� �ʱ�ȭ

    for (int i = 0; i < (int)rankings.size(); ++i)
    {
        const S2CRankingInfo& info = rankings[i];

        CString rankStr, nameStr, levelStr;
        rankStr.Format(_T("%d"), i + 1); // ������ 1���� ����
        nameStr = CString(info.Name);
        levelStr.Format(_T("%d"), info.Level);

        int index = m_rankingListCtrl.InsertItem(i, rankStr); // ����
        m_rankingListCtrl.SetItemText(index, 1, nameStr);     // �̸�
        m_rankingListCtrl.SetItemText(index, 2, levelStr);    // ����
    }

    // �� ���� ���
    CString myRankStr;
    if (myRanking != UINT16_MAX)
        myRankStr.Format(_T("�� ����: %d��"), myRanking);
    else
        myRankStr = _T("�� ����: ����");

    GetDlgItem(IDC_MY_RANKING_STATIC)->SetWindowText(myRankStr);
}
void CRankingDialog::OnBnClickedRankingCloseButton()
{
    PostNcDestroy();
}
