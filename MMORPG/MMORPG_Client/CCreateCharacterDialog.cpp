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
        // ID �ߺ� Ȯ�� ���� ó�� �ݹ� (�� ���� ���)
        m_network->SetCheckNicknameResultCallback([this](uint8_t result)
            {
                if (result == 0)
                    AfxMessageBox(L"��� ������ �г����Դϴ�.");
                else
                    AfxMessageBox(L"�̹� ��� ���� �г����Դϴ�.");
            });

        m_network->SetCreateCharacterResultCallback([this](uint8_t result, uint16_t characterID, uint8_t gender, const char* name)
            {
                if (result == 0)
                {
                    AfxMessageBox(L"ĳ���� ������ �Ϸ�Ǿ����ϴ�.");
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
                    AfxMessageBox(L"ĳ���� ������ �����߽��ϴ�. �ٽ� �õ����ּ���.");
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
        AfxMessageBox(L"�г����� �Է����ּ���.");
        return;
    }

    if (!m_network)
    {
        AfxMessageBox(L"�������� ������ �����ϴ�.");
        return;
    }

    // CString �� std::string
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
        AfxMessageBox(L"�г����� �Է����ּ���.");
        return;
    }
    if (!m_network)
    {
        AfxMessageBox(L"�������� ������ �����ϴ�.");
        return;
    }

    // CString �� std::string
    CT2A asciiConverter(strID);
    std::string nickname(asciiConverter);
	// ���� ���� ��ư
    int gender = 0; // �⺻��: ����
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