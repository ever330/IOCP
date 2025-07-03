#include "CCreateCharacterDialog.h"

IMPLEMENT_DYNAMIC(CCreateCharacterDialog, CDialogEx)

CCreateCharacterDialog::CCreateCharacterDialog(CWnd* pParent)
	: CDialogEx(IDD_CREATE_CHARACTER_DIALOG, pParent)
{
}

CCreateCharacterDialog::~CCreateCharacterDialog()
{
}

void CCreateCharacterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Radio(pDX, IDC_RADIO_MALE, m_gender);
}


BOOL CCreateCharacterDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    if (m_network)
    {
        // ID 중복 확인 응답 처리 콜백 (한 번만 등록)
        m_network->SetCheckNicknameResultCallback([this](uint8_t result)
            {
                if (result == 0)
                    AfxMessageBox(L"사용 가능한 닉네임입니다.");
                else
                    AfxMessageBox(L"이미 사용 중인 닉네임입니다.");
            });

        m_network->SetCreateCharacterResultCallback([this](uint8_t result, uint16_t characterID, uint8_t gender, const char* name)
            {
                if (result == 0)
                {
                    AfxMessageBox(L"캐릭터 생성이 완료되었습니다.");
					Character newCharacter;
					newCharacter.ID = characterID;
					newCharacter.Name = name;
                    newCharacter.GenderType = (Gender)gender;
					newCharacter.Level = 1;
					newCharacter.Experience = 0;
                    m_user->AddCharacter(newCharacter);
                    PostMessage(WM_CREATE_SUCCESS);
                }
                else
                {
                    AfxMessageBox(L"캐릭터 생성에 실패했습니다. 다시 시도해주세요.");
                }
            });
    }

	return TRUE;
}

void CCreateCharacterDialog::SetNetwork(Network* network)
{
    m_network = network;
}

void CCreateCharacterDialog::SetUser(User* user)
{
    m_user = user;
}

BEGIN_MESSAGE_MAP(CCreateCharacterDialog, CDialogEx)
	ON_BN_CLICKED(IDC_NAME_CHECKBUTTON, &CCreateCharacterDialog::OnBnClickedNameCheckbutton)
    ON_BN_CLICKED(IDC_CREATE_BUTTON, &CCreateCharacterDialog::OnBnClickedCreateButton)
    ON_MESSAGE(WM_CREATE_SUCCESS, &CCreateCharacterDialog::OnCreateSuccess)
END_MESSAGE_MAP()

void CCreateCharacterDialog::OnBnClickedNameCheckbutton()
{
    CString strID;
    GetDlgItemText(IDC_NICKNAME, strID);

    if (strID.IsEmpty())
    {
        AfxMessageBox(L"닉네임을 입력해주세요.");
        return;
    }

    if (!m_network)
    {
        AfxMessageBox(L"서버와의 연결이 없습니다.");
        return;
    }

    // CString → std::string
    CT2A asciiConverter(strID);
    std::string nickname(asciiConverter);
    m_network->CharacterNameCheck(nickname);
}

void CCreateCharacterDialog::OnBnClickedCreateButton()
{
    CString strID;
    GetDlgItemText(IDC_NICKNAME, strID);
    if (strID.IsEmpty())
    {
        AfxMessageBox(L"닉네임을 입력해주세요.");
        return;
    }
    if (!m_network)
    {
        AfxMessageBox(L"서버와의 연결이 없습니다.");
        return;
    }

    // CString → std::string
    CT2A asciiConverter(strID);
    std::string nickname(asciiConverter);
	// 성별 라디오 버튼
    int gender = 0; // 기본값: 남성
    if (((CButton*)GetDlgItem(IDC_RADIO_MALE))->GetCheck() == BST_CHECKED)
        gender = 0;
    else if (((CButton*)GetDlgItem(IDC_RADIO_FEMALE))->GetCheck() == BST_CHECKED)
        gender = 1;

    m_network->CreateCharacter(nickname, gender);
}

LRESULT CCreateCharacterDialog::OnCreateSuccess(WPARAM wParam, LPARAM lParam)
{
    EndDialog(IDOK);
    return 0;
}