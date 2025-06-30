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

	DDX_Control(pDX, IDC_SIGNUP_ID, m_idEdit); // IDC_EDIT_ID: ID �Է� ����Ʈ ��Ʈ��
	DDX_Control(pDX, IDC_SIGNUP_PASSWORD, m_passwordEdit); // IDC_EDIT_PASSWORD: ��й�ȣ �Է� ����Ʈ ��Ʈ��
	DDX_Control(pDX, IDC_SIGNUP_PASSWORD_CONFIRM, m_passwordConfirmEdit); // IDC_EDIT_PASSWORD_CONFIRM: ��й�ȣ Ȯ�� �Է� ����Ʈ ��Ʈ��

    m_idEdit.SetLimitText(ID_SIZE);
	m_passwordEdit.SetLimitText(PASSWORD_SIZE);
    m_passwordConfirmEdit.SetLimitText(PASSWORD_SIZE);
}

BOOL CSignUpDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (m_network)
    {
        // ID �ߺ� Ȯ�� ���� ó�� �ݹ� (�� ���� ���)
        m_network->SetCheckIDResultCallback([this](uint8_t result)
            {
                if (result == 0)
                    AfxMessageBox(L"��� ������ ID�Դϴ�.");
                else
                    AfxMessageBox(L"�̹� ��� ���� ID�Դϴ�.");
            });

        m_network->SetSignUpResultCallback([this](uint8_t result)
            {
                if (result == 0)
                {
                    AfxMessageBox(L"ȸ�������� �Ϸ�Ǿ����ϴ�.");
                    PostMessage(WM_SIGNUP_SUCCESS);
                }
                else
                {
                    AfxMessageBox(L"ȸ�����Կ� �����߽��ϴ�. �ٽ� �õ����ּ���.");
                }
			});
    }

    return TRUE;
}

BEGIN_MESSAGE_MAP(CSignUpDialog, CDialogEx)
	ON_BN_CLICKED(IDC_ID_CHECK, &CSignUpDialog::OnClickedIdCheck)
	ON_BN_CLICKED(ID_SIGNUP_SIGNUP, &CSignUpDialog::OnClickedSignupSignup)
    ON_EN_CHANGE(IDC_SIGNUP_ID, &CSignUpDialog::OnEnChangeIDEdit)
    ON_EN_CHANGE(IDC_SIGNUP_PASSWORD, &CSignUpDialog::OnEnChangePasswordEdit)
    ON_EN_CHANGE(IDC_SIGNUP_PASSWORD_CONFIRM, &CSignUpDialog::OnEnChangePasswordConfirmEdit)
    ON_MESSAGE(WM_SIGNUP_SUCCESS, &CSignUpDialog::OnSignUpSuccess)
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

    // CString �� std::string
    CT2A asciiConverter(strID);
    std::string username(asciiConverter);
    m_network->IDCheck(username);
}

void CSignUpDialog::OnClickedSignupSignup()
{
    CString strID, strPassword, strPasswordConfirm;
    GetDlgItemText(IDC_SIGNUP_ID, strID); // IDC_EDIT_ID: ID �Է� ����Ʈ ��Ʈ��
    GetDlgItemText(IDC_SIGNUP_PASSWORD, strPassword); // IDC_EDIT_PASSWORD: ��й�ȣ �Է� ����Ʈ ��Ʈ��
    GetDlgItemText(IDC_SIGNUP_PASSWORD_CONFIRM, strPasswordConfirm); // IDC_EDIT_PASSWORD_CONFIRM: ��й�ȣ Ȯ�� �Է� ����Ʈ ��Ʈ��

    if (strID.IsEmpty() || strPassword.IsEmpty() || strPasswordConfirm.IsEmpty())
    {
        AfxMessageBox(L"��� �ʵ带 �Է����ּ���.");
        return;
    }
    if (strPassword != strPasswordConfirm)
    {
        AfxMessageBox(L"��й�ȣ�� ��ġ���� �ʽ��ϴ�.");
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
	m_network->SignUp(username, password);
}

void CSignUpDialog::OnEnChangeIDEdit()
{
    CString strID;
    m_idEdit.GetWindowText(strID);

    CString filteredID;
    for (int i = 0; i < strID.GetLength(); ++i)
    {
        TCHAR ch = strID[i];
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9'))
        {
            filteredID.AppendChar(ch);
        }
    }

    // ��ȿ���� ���� ���ڰ� �־����� �ٽ� ����
    if (strID != filteredID)
    {
        int nPos = m_idEdit.GetSel(); // ���� Ŀ�� ��ġ ���
        m_idEdit.SetWindowText(filteredID);
        m_idEdit.SetSel(nPos, nPos); // Ŀ�� ��ġ ����
    }
}

void CSignUpDialog::OnEnChangePasswordEdit()
{
    CString strPwd;
    m_passwordEdit.GetWindowText(strPwd);

    CString filteredPwd;
    for (int i = 0; i < strPwd.GetLength(); ++i)
    {
        TCHAR ch = strPwd[i];
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            _tcschr(_T("!@#$%^&*()-_=+[]{};:'\",.<>?/|\\~`"), ch))
        {
            filteredPwd.AppendChar(ch);
        }
    }

    if (strPwd != filteredPwd)
    {
        int nPos = m_passwordEdit.GetSel(); // ���� Ŀ�� ��ġ ���
        m_passwordEdit.SetWindowText(filteredPwd);
        m_passwordEdit.SetSel(nPos, nPos); // Ŀ�� ��ġ ����
    }
}

void CSignUpDialog::OnEnChangePasswordConfirmEdit()
{
    CString strPwd;
    m_passwordConfirmEdit.GetWindowText(strPwd);

    CString filteredPwd;
    for (int i = 0; i < strPwd.GetLength(); ++i)
    {
        TCHAR ch = strPwd[i];
        if ((ch >= 'A' && ch <= 'Z') ||
            (ch >= 'a' && ch <= 'z') ||
            (ch >= '0' && ch <= '9') ||
            _tcschr(_T("!@#$%^&*()-_=+[]{};:'\",.<>?/|\\~`"), ch))
        {
            filteredPwd.AppendChar(ch);
        }
    }

    if (strPwd != filteredPwd)
    {
        int nPos = m_passwordConfirmEdit.GetSel();
        m_passwordConfirmEdit.SetWindowText(filteredPwd);
        m_passwordConfirmEdit.SetSel(nPos, nPos);
    }
}

LRESULT CSignUpDialog::OnSignUpSuccess(WPARAM wParam, LPARAM lParam)
{
    EndDialog(IDOK);
    return 0;
}
