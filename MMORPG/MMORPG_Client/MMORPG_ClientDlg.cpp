// MMORPG_ClientDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MMORPG_Client.h"
#include "MMORPG_ClientDlg.h"
#include "afxdialogex.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMMORPGClientDlg 대화 상자



CMMORPGClientDlg::CMMORPGClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MMORPG_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMMORPGClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMMORPGClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SEND_BUTTON, &CMMORPGClientDlg::OnSendButtonClicked)
	ON_BN_CLICKED(IDC_MOVE_BUTTON, &CMMORPGClientDlg::OnMoveButtonClicked)
END_MESSAGE_MAP()


// CMMORPGClientDlg 메시지 처리기

#include <string> // Add this include for std::string and std::wstring conversion

#include <atlstr.h> // Include this for CString conversion

BOOL CMMORPGClientDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetChatIO();
    SetMapList();
    SetGameView();
    SetCallback();
    m_userNicknameStatic.SubclassDlgItem(IDC_NICKNAME, this);

    CString userNickname = CString(m_user->GetUsername().c_str());
    m_userNicknameStatic.SetWindowText(userNickname);

    m_testView->SetNetwork(m_network);
	m_testView->SetUser(m_user);

    // IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
    //  프레임워크가 이 작업을 자동으로 수행합니다.
    SetIcon(m_hIcon, TRUE);         // 큰 아이콘을 설정합니다.
    SetIcon(m_hIcon, FALSE);        // 작은 아이콘을 설정합니다.

    // TODO: 여기에 추가 초기화 작업을 추가합니다.

    return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMMORPGClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMMORPGClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMMORPGClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMMORPGClientDlg::OnSendButtonClicked()
{
	CString strInput;
	m_chatInput.GetWindowText(strInput);

	if (strInput.IsEmpty())
	{
		AfxMessageBox(L"내용을 입력해주세요.");
		return;
	}

	CT2A asciiConverter(strInput);
	std::string stdInput(asciiConverter);

	m_network->SendChat(stdInput);

	// Clear the input box  
	m_chatInput.SetWindowText(L"");
}

void CMMORPGClientDlg::AddChatMessage(const CString& message)
{
	int nLength = m_chatOutput.GetWindowTextLength();
	m_chatOutput.SetSel(nLength, nLength);
	m_chatOutput.ReplaceSel(message + L"\r\n");
	m_chatOutput.SetSel(-1, -1);
	m_chatOutput.LineScroll(1);
}

void CMMORPGClientDlg::OnMoveButtonClicked()
{
	int selIndex = m_mapList.GetCurSel();
	if (selIndex == CB_ERR)
	{
		AfxMessageBox(L"이동할 맵을 선택해주세요.");
		return;
	}

	CString selectedMap;
	m_mapList.GetLBText(selIndex, selectedMap);

	unsigned int mapID = _ttoi(selectedMap);

	CString msg;
	msg.Format(L"선택된 맵: %s (맵 ID: %u)", (LPCTSTR)selectedMap, mapID);
	AfxMessageBox(msg);

	m_network->ChangeMap(mapID);
}

BOOL CMMORPGClientDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// 엔터 누르면 채팅 전송 + 포커스 제거
		if (pMsg->wParam == VK_RETURN)
		{
			if (GetFocus() == &m_chatInput && m_chatInput.m_bAllowFocus)
			{
				OnSendButtonClicked();
				m_chatInput.m_bAllowFocus = FALSE;
				GetDlgItem(IDC_GAME_VIEW)->SetFocus();
				return TRUE;
			}
		}

		// 방향키도 채팅창에 있으면 게임 뷰로 포커스 이동
		if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT ||
			pMsg->wParam == VK_UP || pMsg->wParam == VK_DOWN)
		{
			if (GetFocus() == &m_chatInput)
			{
				GetDlgItem(IDC_GAME_VIEW)->SetFocus();
				return TRUE;
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CMMORPGClientDlg::OnDestroy()
{
	if (m_testView)
	{
		m_testView->DestroyWindow();
		m_testView = nullptr;
	}

	CDialogEx::OnDestroy();
}

void CMMORPGClientDlg::SetChatIO()
{
	m_chatOutput.SubclassDlgItem(IDC_CHAT_OUTPUT, this);
	m_chatInput.SubclassDlgItem(IDC_CHAT_INPUT, this);
	m_chatInput.SetParent(this); // 부모 대화상자 연결
	m_sendButton.SubclassDlgItem(IDC_SEND_BUTTON, this);
}

void CMMORPGClientDlg::SetMapList()
{
	m_mapList.SubclassDlgItem(IDC_MAP_LIST, this);
	m_moveButton.SubclassDlgItem(IDC_MOVE_BUTTON, this);

	m_mapNameStatic.SubclassDlgItem(IDC_MAP_NAME, this);

	m_mapList.AddString(L"1001");
	m_mapList.AddString(L"1002");
	m_mapList.AddString(L"1003");
	m_mapList.AddString(L"2001");
	m_mapList.AddString(L"2002");
	m_mapList.AddString(L"3001");
	m_mapList.AddString(L"3002");
	m_mapList.AddString(L"4001");
	m_mapList.AddString(L"4002");
	m_mapList.AddString(L"4003");
	m_mapList.AddString(L"5001");
}

void CMMORPGClientDlg::SetGameView()
{
	CRect rect;
	GetDlgItem(IDC_GAME_VIEW)->GetWindowRect(&rect);
	ScreenToClient(&rect);

	m_testView = new CTestView();
	m_testView->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, 1234);
	m_testView->OnInitialUpdate();
	m_testView->ShowWindow(SW_SHOW);
}

void CMMORPGClientDlg::SetCallback()
{
	m_network->SetMessageCallback([this](const std::string& message) {
		CString cstrMessage(message.c_str());
		AddChatMessage(cstrMessage);
		});

	m_network->SetMapChangeCallback([this](uint16_t mapID, float x, float y) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			::SendMessage(m_testView->GetSafeHwnd(), WM_MAP_CHANGE, x, y);
		}

		CString mapName;
		switch (mapID)
		{
		case 1001: mapName = L"초원1"; break;
		case 1002: mapName = L"초원2"; break;
		case 1003: mapName = L"초원3"; break;
		case 2001: mapName = L"숲1"; break;
		case 2002: mapName = L"숲2"; break;
		case 3001: mapName = L"바다1"; break;
		case 3002: mapName = L"바다2"; break;
		case 4001: mapName = L"산1"; break;
		case 4002: mapName = L"산2"; break;
		case 4003: mapName = L"산3"; break;
		case 5001: mapName = L"던전"; break;
		default: mapName = L"알 수 없는 맵"; break;
		}

		m_mapNameStatic.SetWindowText(mapName);

		CString msg;
		msg.Format(L"[맵 이동] %s (맵 ID: %d) 위치: (%f, %f)", (LPCTSTR)mapName, mapID, x, y);
		AddChatMessage(msg);
		});

	m_network->SetMonsterInfoCallback([this](const std::vector<S2CMonsterStateInfo>& monsters) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			auto* pCopy = new std::vector<S2CMonsterStateInfo>(monsters);
			::SendMessage(m_testView->GetSafeHwnd(), WM_UPDATE_MONSTER_STATE, 0, reinterpret_cast<LPARAM>(pCopy));
		}
		});

	m_network->SetPlayerEnterCallback([this](uint16_t userID, const char* name, float x, float y) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			Vector3 position{ x, y, 0.0f };
			auto* pPlayer = new OtherPlayer{ userID, CString(name), position };
			::SendMessage(m_testView->GetSafeHwnd(), WM_PLAYER_ENTER, 0, reinterpret_cast<LPARAM>(pPlayer));
		}
		});

	m_network->SetPlayerLeaveCallback([this](uint16_t userID) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			::SendMessage(m_testView->GetSafeHwnd(), WM_PLAYER_LEAVE, static_cast<WPARAM>(userID), 0);
		}
		});

	m_network->SetPlayerInfoCallback([this](const std::vector<S2CPlayerStateInfo>& users) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			auto* pCopy = new std::vector<S2CPlayerStateInfo>(users);
			::SendMessage(m_testView->GetSafeHwnd(), WM_UPDATE_PLAYER_STATE, 0, reinterpret_cast<LPARAM>(pCopy));
		}
		});

	m_network->SetPlayerMoveCallback([this](uint16_t userID, Direction direction) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			::SendMessage(m_testView->GetSafeHwnd(), WM_OTHER_PLAYER_MOVE, static_cast<WPARAM>(userID), static_cast<LPARAM>(direction));
		}
		});

	m_network->SetPlayerPosSyncCallback([this](float x, float y, float z, uint32_t frameID) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			Vector3 position{ x, y, z };
			::SendMessage(m_testView->GetSafeHwnd(), WM_PLAYER_POS_SYNC, static_cast<WPARAM>(frameID), *reinterpret_cast<LPARAM*>(&position));
		}
		});

	m_network->SetMonsterHitInfoCallback([this](const std::vector<S2CMonsterHitInfo>& monsters) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			auto* pCopy = new std::vector<S2CMonsterHitInfo>(monsters);
			::SendMessage(m_testView->GetSafeHwnd(), WM_UPDATE_MONSTER_HIT, 0, reinterpret_cast<LPARAM>(pCopy));
		}
		});

	m_network->SetMonsterRespawnCallback([this](const std::vector<S2CMonsterRespawnInfo>& monsters) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			auto* pCopy = new std::vector<S2CMonsterRespawnInfo>(monsters);
			::SendMessage(m_testView->GetSafeHwnd(), WM_MONSTER_RESPAWN, 0, reinterpret_cast<LPARAM>(pCopy));
		}
		});

	m_network->SetPlayerAttackCallback([this](uint16_t userID, Direction dir) {
		if (m_testView && ::IsWindow(m_testView->GetSafeHwnd()))
		{
			::SendMessage(m_testView->GetSafeHwnd(), WM_PLAYER_ATTACK, static_cast<WPARAM>(userID), static_cast<LPARAM>(dir));
		}
		});
}

void CMMORPGClientDlg::OnClose()
{
	m_network->SetMessageCallback(nullptr);
	m_network->SetMapChangeCallback(nullptr);
	m_network->SetMonsterInfoCallback(nullptr);
	m_network->SetPlayerEnterCallback(nullptr);
	m_network->SetPlayerLeaveCallback(nullptr);

	CDialogEx::OnClose();
}

void CMMORPGClientDlg::OnCancel()
{
	// 네트워크 콜백 해제 및 연결 종료를 예외 안전하게 처리
	try
	{
		m_network->SetMessageCallback(nullptr);
		m_network->SetMapChangeCallback(nullptr);
		m_network->SetMonsterInfoCallback(nullptr);
		m_network->SetPlayerEnterCallback(nullptr);
		m_network->SetPlayerLeaveCallback(nullptr);
	}
	catch (...)
	{
		// 예외 발생 시 무시(종료 과정에서의 예외 방지)
	}

	CDialogEx::OnCancel();
}