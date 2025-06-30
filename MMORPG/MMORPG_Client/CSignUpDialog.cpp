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

	DDX_Control(pDX, IDC_SIGNUP_ID, m_idEdit); // IDC_EDIT_ID: ID 입력 에디트 컨트롤
	DDX_Control(pDX, IDC_SIGNUP_PASSWORD, m_passwordEdit); // IDC_EDIT_PASSWORD: 비밀번호 입력 에디트 컨트롤
	DDX_Control(pDX, IDC_SIGNUP_PASSWORD_CONFIRM, m_passwordConfirmEdit); // IDC_EDIT_PASSWORD_CONFIRM: 비밀번호 확인 입력 에디트 컨트롤

    m_idEdit.SetLimitText(ID_SIZE);
	m_passwordEdit.SetLimitText(PASSWORD_SIZE);
    m_passwordConfirmEdit.SetLimitText(PASSWORD_SIZE);
}

BOOL CSignUpDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    if (m_network)
    {
        // ID 중복 확인 응답 처리 콜백 (한 번만 등록)
        m_network->SetCheckIDResultCallback([this](uint8_t result)
            {
                if (result == 0)
                    AfxMessageBox(L"사용 가능한 ID입니다.");
                else
                    AfxMessageBox(L"이미 사용 중인 ID입니다.");
            });

        m_network->SetSignUpResultCallback([this](uint8_t result)
            {
                if (result == 0)
                {
                    AfxMessageBox(L"회원가입이 완료되었습니다.");
                    PostMessage(WM_SIGNUP_SUCCESS);
                }
                else
                {
                    AfxMessageBox(L"회원가입에 실패했습니다. 다시 시도해주세요.");
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

    // CString → std::string
    CT2A asciiConverter(strID);
    std::string username(asciiConverter);
    m_network->IDCheck(username);
}

void CSignUpDialog::OnClickedSignupSignup()
{
    CString strID, strPassword, strPasswordConfirm;
    GetDlgItemText(IDC_SIGNUP_ID, strID); // IDC_EDIT_ID: ID 입력 에디트 컨트롤
    GetDlgItemText(IDC_SIGNUP_PASSWORD, strPassword); // IDC_EDIT_PASSWORD: 비밀번호 입력 에디트 컨트롤
    GetDlgItemText(IDC_SIGNUP_PASSWORD_CONFIRM, strPasswordConfirm); // IDC_EDIT_PASSWORD_CONFIRM: 비밀번호 확인 입력 에디트 컨트롤

    if (strID.IsEmpty() || strPassword.IsEmpty() || strPasswordConfirm.IsEmpty())
    {
        AfxMessageBox(L"모든 필드를 입력해주세요.");
        return;
    }
    if (strPassword != strPasswordConfirm)
    {
        AfxMessageBox(L"비밀번호가 일치하지 않습니다.");
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

    // 유효하지 않은 문자가 있었으면 다시 세팅
    if (strID != filteredID)
    {
        int nPos = m_idEdit.GetSel(); // 현재 커서 위치 기억
        m_idEdit.SetWindowText(filteredID);
        m_idEdit.SetSel(nPos, nPos); // 커서 위치 복원
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
        int nPos = m_passwordEdit.GetSel(); // 현재 커서 위치 기억
        m_passwordEdit.SetWindowText(filteredPwd);
        m_passwordEdit.SetSel(nPos, nPos); // 커서 위치 복원
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
