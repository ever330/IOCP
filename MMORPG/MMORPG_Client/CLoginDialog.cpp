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

    m_idEdit.SetLimitText(ID_SIZE);                 // ID �ִ� ���� ����
    m_passwordEdit.SetLimitText(PASSWORD_SIZE);     // ��й�ȣ �ִ� ���� ����
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
                    AfxMessageBox(L"�α��ο� �����߽��ϴ�. ���̵� �Ǵ� ��й�ȣ�� Ȯ�����ּ���.");
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
	signUpDlg.SetNetwork(m_network);  // ���� Network�� �ʿ��ϴٸ� ������ ����
	signUpDlg.DoModal();               // ��� ������� ���̾�α� ����
}

void CLoginDialog::OnClickedLogin()
{
    CString strID, strPassword;
    GetDlgItemText(IDC_LOGIN_ID, strID); // IDC_EDIT_ID: ID �Է� ����Ʈ ��Ʈ��
    GetDlgItemText(IDC_LOGIN_PASSWORD, strPassword); // IDC_EDIT_PASSWORD: ��й�ȣ �Է� ����Ʈ ��Ʈ��

    if (strID.IsEmpty() || strPassword.IsEmpty())
    {
        AfxMessageBox(L"��� �ʵ带 �Է����ּ���.");
        return;
    }
    if (!m_network)
    {
        AfxMessageBox(L"�������� ������ �����ϴ�.");
        return;
    }

    // CString �� std::string
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
