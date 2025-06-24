#include "pch.h"
#include "MMORPG_Client.h"
#include "afxdialogex.h"
#include "CLoginDialog.h"
#include "CSignUpDialog.h"

IMPLEMENT_DYNAMIC(CSignUpDialog, CDialogEx)

CSignUpDialog::CSignUpDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SIGNUP_DIALOG, pParent)
{

}

CSignUpDialog::~CSignUpDialog()
{
}

void CSignUpDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CSignUpDialog::OnBnClickedBtnCheckId()
{

}

void CSignUpDialog::OnBnClickedBtnSignUp()
{

}


BEGIN_MESSAGE_MAP(CSignUpDialog, CDialogEx)
	ON_BN_CLICKED(IDC_ID_CHECK, &CSignUpDialog::OnClickedIdCheck)
	ON_BN_CLICKED(ID_SIGNUP_SIGNUP, &CSignUpDialog::OnClickedSignupSignup)
	ON_BN_CLICKED(ID_SIGNUP_CANCLE, &CSignUpDialog::OnBnClickedSignupCancle)
END_MESSAGE_MAP()

void CSignUpDialog::OnClickedIdCheck()
{
    CString strID;
    GetDlgItemText(IDC_SIGNUP_ID, strID); // IDC_EDIT_ID: ID �Է� ����Ʈ ��Ʈ��

    if (strID.IsEmpty())
    {
        AfxMessageBox(L"ID�� �Է����ּ���.");
        return;
    }

    if (!m_network)
    {
        AfxMessageBox(L"�������� ������ �����ϴ�.");
        return;
    }

    m_network->SetCheckIDResultCallback([this](uint8_t result)
        {
            if (result == 0)
                AfxMessageBox(L"��� ������ ID�Դϴ�.");
            else
                AfxMessageBox(L"�̹� ��� ���� ID�Դϴ�.");
        });

    // CString �� std::string
    CT2A asciiConverter(strID);
    std::string username(asciiConverter);
    m_network->IDCheck(username);

    AfxMessageBox(L"�ߺ� Ȯ�� ��û�� ������ �����߽��ϴ�.");
}

void CSignUpDialog::OnClickedSignupSignup()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}

void CSignUpDialog::OnBnClickedSignupCancle()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}
