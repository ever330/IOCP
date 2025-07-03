#include "CCharacterSelectDialog.h"
#include "CRankingDialog.h"

IMPLEMENT_DYNAMIC(CCharacterSelectDialog, CDialogEx)

CCharacterSelectDialog::CCharacterSelectDialog(CWnd* pParent)
    : CDialogEx(IDD_SELECT_CHARACTER_DIALOG, pParent)
{
}

CCharacterSelectDialog::~CCharacterSelectDialog()
{
}

void CCharacterSelectDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_CHARACTER_LIST, m_characterList);
    DDX_Control(pDX, IDC_CHARACTER_INFO, m_characterInfo);
}

BOOL CCharacterSelectDialog::OnInitDialog()  
{  
    CDialogEx::OnInitDialog();  

    if (m_network)
    {
        m_network->SetSelectCharacterResultCallback([this](uint8_t result, uint16_t characterId, const char* name, const uint16_t level, uint32_t exp)
            {
                if (result == 0)
                {
					Character character;
					character.ID = characterId;
                    character.Name = name;
					character.Level = level;
					character.Experience = exp;

					m_user->SetActiveCharacter(character);

                    // ĳ���� ���� ����. �ΰ��� ȭ������ ��ȯ
                    PostMessage(WM_CHARACTER_SELECT_SUCCESS);
                }
                else
                {
                    AfxMessageBox(L"�α��ο� �����߽��ϴ�. ���̵� �Ǵ� ��й�ȣ�� Ȯ�����ּ���.");
                }
            });
    }

	RefreshCharacterList();

    return TRUE;  
}

BEGIN_MESSAGE_MAP(CCharacterSelectDialog, CDialogEx)
	ON_BN_CLICKED(IDC_GAMESTART, &CCharacterSelectDialog::OnBnClickedGamestart)
    ON_BN_CLICKED(IDC_CREATE_CHARACTER, &CCharacterSelectDialog::OnBnClickedCreateCharacter)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_CHARACTER_LIST, &CCharacterSelectDialog::OnLvnItemchangedCharacterList)
    ON_MESSAGE(WM_CHARACTER_SELECT_SUCCESS, &CCharacterSelectDialog::OnCharacterSelectAck)
END_MESSAGE_MAP()

void CCharacterSelectDialog::OnBnClickedGamestart()
{
    int selectedIndex = m_characterList.GetNextItem(-1, LVNI_SELECTED);
    if (selectedIndex == -1)
    {
        AfxMessageBox(L"ĳ���͸� �������ּ���.");
        return;
    }
    uint16_t charID = m_characterIndexMap[selectedIndex];
    if (m_network)
    {
        m_network->SelectCharacter(charID);
    }
    else
    {
        AfxMessageBox(L"�������� ������ ���������ϴ�.");
	}
}

void CCharacterSelectDialog::OnBnClickedCreateCharacter()
{
    CCreateCharacterDialog createCharacterDialog;
	createCharacterDialog.SetUser(m_user);
	createCharacterDialog.SetNetwork(m_network);

    if (createCharacterDialog.DoModal())
    {
		RefreshCharacterList();
    }
}

void CCharacterSelectDialog::OnLvnItemchangedCharacterList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pListView = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    int index = pListView->iItem;

    if (index >= 0 && index < m_characterIndexMap.size() &&
        (pListView->uChanged & LVIF_STATE) && (pListView->uNewState & LVIS_SELECTED))
    {
        uint16_t charID = m_characterIndexMap[index];
        const auto& characterMap = m_user->GetCharacters();
        auto it = characterMap.find(charID);
        if (it != characterMap.end())
        {
            const auto& ch = it->second; 
            CString info;
            CString genderStr = ((int)ch.GenderType == 0) ? _T("����") : _T("����");

            info.Format(_T("����: %s\n����: %d\n����ġ: %u"),
                genderStr,
                ch.Level,
                ch.Experience);

            m_characterInfo.SetWindowText(info);
        }
    }

    *pResult = 0;
}

LRESULT CCharacterSelectDialog::OnCharacterSelectAck(WPARAM wParam, LPARAM lParam)
{
    EndDialog(IDOK);
	return 0;
}

void CCharacterSelectDialog::SetUser(User* user)
{
	m_user = user;
}

void CCharacterSelectDialog::SetNetwork(Network* network)
{
    m_network = network;
}

void CCharacterSelectDialog::RefreshCharacterList()
{
    const auto& characters = m_user->GetCharacters();
    m_characterIndexMap.clear();
    m_characterList.DeleteAllItems();

    int i = 0;
    for (const auto& it : characters)
    {
        CString name(it.second.Name.c_str());
        m_characterList.InsertItem(i, name);
        m_characterIndexMap.push_back(it.second.ID);
        ++i;
    }
}