// CNicknameDialog.cpp: 구현 파일
//

#include "pch.h"
#include "MMORPG_Client.h"
#include "afxdialogex.h"
#include "CNicknameDialog.h"


// CNicknameDialog 대화 상자

IMPLEMENT_DYNAMIC(CNicknameDialog, CDialogEx)

CNicknameDialog::CNicknameDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NICKNAME_DIALOG, pParent)
{

}

CNicknameDialog::~CNicknameDialog()
{
}

void CNicknameDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strNickname);
}


BEGIN_MESSAGE_MAP(CNicknameDialog, CDialogEx)
END_MESSAGE_MAP()


// CNicknameDialog 메시지 처리기
