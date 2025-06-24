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
	signUpDlg.SetNetwork(m_network);  // 만약 Network가 필요하다면 포인터 전달
	signUpDlg.DoModal();               // 모달 방식으로 다이얼로그 실행
}

void CLoginDialog::OnClickedLogin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
