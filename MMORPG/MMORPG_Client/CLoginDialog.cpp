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

void CLoginDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CLoginDialog, CDialogEx)
	ON_BN_CLICKED(ID_LOGIN_SIGNUP, &CLoginDialog::OnClickedLoginSignup)
	ON_BN_CLICKED(ID_LOGIN, &CLoginDialog::OnClickedLogin)
END_MESSAGE_MAP()
void CLoginDialog::OnClickedLoginSignup()
{
	CSignUpDialog signUpDlg;
	signUpDlg.SetNetwork(m_network);  // ���� Network�� �ʿ��ϴٸ� ������ ����
	signUpDlg.DoModal();               // ��� ������� ���̾�α� ����
}

void CLoginDialog::OnClickedLogin()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}
