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
    GetDlgItemText(IDC_SIGNUP_ID, strID); // IDC_EDIT_ID: ID 입력 에디트 컨트롤

    if (strID.IsEmpty())
    {
        AfxMessageBox(L"ID를 입력해주세요.");
        return;
    }

    if (!m_network)
    {
        AfxMessageBox(L"서버와의 연결이 없습니다.");
        return;
    }

    m_network->SetCheckIDResultCallback([this](uint8_t result)
        {
            if (result == 0)
                AfxMessageBox(L"사용 가능한 ID입니다.");
            else
                AfxMessageBox(L"이미 사용 중인 ID입니다.");
        });

    // CString → std::string
    CT2A asciiConverter(strID);
    std::string username(asciiConverter);
    m_network->IDCheck(username);

    AfxMessageBox(L"중복 확인 요청을 서버로 전송했습니다.");
}

void CSignUpDialog::OnClickedSignupSignup()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CSignUpDialog::OnBnClickedSignupCancle()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
