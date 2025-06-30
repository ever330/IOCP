#include "pch.h"
#include "MMORPG_Client.h"
#include "afxdialogex.h"
#include "CLoginDialog.h"
#include "CSignUpDialog.h"

IMPLEMENT_DYNAMIC(CLoginDialog, CDialogEx)

CLoginDialog::CLoginDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LOGIN_DIALOG, pParent)
{

}

CLoginDialog::~CLoginDialog()
{
}

void CLoginDialog::SetNetwork(Network* network)
{
    m_network = network;
}

void CLoginDialog::SetUser(User* user)
{
    m_user = user;
}

void CLoginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_LOGIN_ID, m_idEdit);
    DDX_Control(pDX, IDC_LOGIN_PASSWORD, m_passwordEdit);

    m_idEdit.SetLimitText(ID_SIZE);                 // ID 최대 길이 제한
    m_passwordEdit.SetLimitText(PASSWORD_SIZE);     // 비밀번호 최대 길이 제한
}

BOOL CLoginDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (m_network)
    {
        m_network->SetLoginResultCallback([this](uint8_t result, uint16_t userId, const char* id)
            {
                if (result == 0)
                {
					m_user->SetUserId(userId);
					std::string idStr(id);
					m_user->SetUsername(idStr);
                    PostMessage(WM_LOGIN_SUCCESS);
                }
                else
                {
                    AfxMessageBox(L"로그인에 실패했습니다. 아이디 또는 비밀번호를 확인해주세요.");
                }
            });
    }

    return TRUE;
}

BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
	ON_BN_CLICKED(ID_LOGIN_SIGNUP, &CLoginDialog::OnClickedLoginSignup)
	ON_BN_CLICKED(ID_LOGIN, &CLoginDialog::OnClickedLogin)
	ON_MESSAGE(WM_LOGIN_SUCCESS, &CLoginDialog::OnLoginSuccess)
END_MESSAGE_MAP()
void CLoginDialog::OnClickedLoginSignup()
{
	CSignUpDialog signUpDlg;
	signUpDlg.SetNetwork(m_network);  // 만약 Network가 필요하다면 포인터 전달
	signUpDlg.DoModal();               // 모달 방식으로 다이얼로그 실행
}

void CLoginDialog::OnClickedLogin()
{
    CString strID, strPassword;
    GetDlgItemText(IDC_LOGIN_ID, strID); // IDC_EDIT_ID: ID 입력 에디트 컨트롤
    GetDlgItemText(IDC_LOGIN_PASSWORD, strPassword); // IDC_EDIT_PASSWORD: 비밀번호 입력 에디트 컨트롤

    if (strID.IsEmpty() || strPassword.IsEmpty())
    {
        AfxMessageBox(L"모든 필드를 입력해주세요.");
        return;
    }
    if (!m_network)
    {
        AfxMessageBox(L"서버와의 연결이 없습니다.");
        return;
    }

    // CString → std::string
    CT2A asciiConverterID(strID);
    std::string username(asciiConverterID);

    CT2A asciiConverterPwd(strPassword);
    std::string password(asciiConverterPwd);
    m_network->Login(username, password);
}

LRESULT CLoginDialog::OnLoginSuccess(WPARAM wParam, LPARAM lParam)
{
    EndDialog(IDOK);
    return 0;
}
