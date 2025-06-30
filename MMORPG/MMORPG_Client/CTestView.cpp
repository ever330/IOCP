#include "CTestView.h"
#include "Network.h"
#include "Vector3.h"

BEGIN_MESSAGE_MAP(CTestView, CView)
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_UPDATE_MONSTER_STATE, &CTestView::OnUpdateMonsterState)
	ON_MESSAGE(WM_MAP_CHANGE, &CTestView::OnMapChange)
	ON_MESSAGE(WM_PLAYER_ENTER, &CTestView::OnPlayerEnter)
	ON_MESSAGE(WM_PLAYER_LEAVE, &CTestView::OnPlayerLeave)
	ON_MESSAGE(WM_UPDATE_PLAYER_STATE, &CTestView::OnUpdatePlayerState)
	ON_MESSAGE(WM_OTHER_PLAYER_MOVE, &CTestView::OnOtherPlayerMove)
	ON_MESSAGE(WM_PLAYER_POS_SYNC, &CTestView::OnPlayerPosSync)
	ON_MESSAGE(WM_UPDATE_MONSTER_HIT, &CTestView::OnUpdateMonsterHit)
	ON_MESSAGE(WM_MONSTER_RESPAWN, &CTestView::OnRespawnMonster)
	ON_MESSAGE(WM_PLAYER_ATTACK, &CTestView::OnPlayerAttack)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CTestView::CTestView()
{
	// �ʱ� ��ġ ����
	m_playerPos = CPoint(100, 100);
	m_network = nullptr;
	m_lastAttackTime = 0;
	m_attackStartTime = 0;
	m_isAttacking = false;
	m_isMoving = false;

	m_attackDirection = Direction::Up;
	m_playerDirection = Direction::Up;
	m_user = nullptr;

	m_currentFrameID = 0;
}

CTestView::~CTestView()
{
}

BOOL CTestView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;  // ������ ���� ȿ��
}

void CTestView::OnDraw(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);

	// ���� ���� ���� �޸� DC�� �׸���
	DrawScene(&memDC);

	// ���������� ȭ�鿡 ����
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBitmap);
}

void CTestView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	UINT_PTR timerID = SetTimer(1, 100, nullptr);
	if (timerID == 0) {
		AfxMessageBox(_T("Ÿ�̸� ���� ����!"));
	}
	m_lastUpdateTime = std::chrono::steady_clock::now();
}

void CTestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	DWORD now = GetTickCount64();

	// �����̽���(����)�� �̵� �Ǵ� ���� ���� ó��
	if (nChar == VK_SPACE)
	{
		if (now - m_lastAttackTime > 500)
		{
			m_lastAttackTime = now;

			m_isAttacking = true;
			m_attackStartTime = now;
			m_attackStartPos = m_playerPos;
			m_attackDirection = m_playerDirection;

			if (m_network)
			{
				m_network->PlayerStop(m_currentFrameID++);
				m_isMoving = false;
				m_network->PlayerAttack(m_playerDirection);
			}
		}
		Invalidate(FALSE);
		return; // ���⼭ ���� (�Ʒ� �̵� ���� ���� X)
	}

	switch (nChar)
	{
	case VK_LEFT:  m_playerDirection = Direction::Left; break;
	case VK_RIGHT: m_playerDirection = Direction::Right; break;
	case VK_UP:    m_playerDirection = Direction::Up; break;
	case VK_DOWN:  m_playerDirection = Direction::Down; break;
	default: return;
	}

	m_isMoving = true;

	// ���� ����
	if (m_network)
	{
		uint32_t frameID = m_currentFrameID++;
		m_inputBuffer.push_back({ frameID, m_playerDirection });
		m_network->PlayerMove(m_playerDirection, frameID);
	}

	Invalidate(FALSE);
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTestView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	Direction releasedDir;
	switch (nChar)
	{
	case VK_LEFT:  releasedDir = Direction::Left; break;
	case VK_RIGHT: releasedDir = Direction::Right; break;
	case VK_UP:    releasedDir = Direction::Up; break;
	case VK_DOWN:  releasedDir = Direction::Down; break;
	default: return;
	}

	if (releasedDir == m_playerDirection)
	{
		m_isMoving = false;
		if (m_network)
			m_network->PlayerStop(m_currentFrameID);
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CTestView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CFrameWnd* pFrame = GetParentFrame();
	if (pFrame)
	{
		pFrame->SetActiveView(this);
	}

	SetFocus();

	CView::OnLButtonDown(nFlags, point);
}

UINT CTestView::OnGetDlgCode()
{
	return DLGC_WANTARROWS | CView::OnGetDlgCode();
}

void CTestView::DrawScene(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	// ���
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	// ����
	for (auto& monster : m_monsters)
	{
		monster.second->Render(pDC);
	}

	// �ٸ� �÷��̾��
	for (const auto& player : m_otherPlayers)
	{
		int x = static_cast<int>(player.second.position.x);
		int y = static_cast<int>(player.second.position.y);

		// ���� ũ�� ����
		CString name = player.second.name;
		CSize textSize = pDC->GetTextExtent(name);

		// ��� �簢�� ���
		CRect bgRect(x - 10, y - 25, x - 10 + textSize.cx, y - 25 + textSize.cy);
		pDC->FillSolidRect(bgRect, RGB(255, 255, 255));

		// �ؽ�Ʈ�� ���� ������� ���
		pDC->SetBkMode(TRANSPARENT);
		pDC->TextOut(x - 10, y - 25, name);
		pDC->FillSolidRect(x - 10, y - 10, 20, 20, RGB(0, 200, 0));
	}

	// �� �÷��̾�
	pDC->FillSolidRect(m_playerPos.x - 10, m_playerPos.y - 10, 20, 20, RGB(0, 0, 255));

	DWORD now = GetTickCount64();
	// ���� ���
	if (m_isAttacking)
	{
		if (now - m_attackStartTime < 200)
		{
			DrawAttackEffect(pDC, m_attackStartPos, m_attackDirection);
		}
		else
		{
			m_isAttacking = false;
		}
	}

	// �ٸ� �÷��̾��� ���� ���
	for (auto it = m_otherAttacks.begin(); it != m_otherAttacks.end(); )
	{
		if (now - it->second.startTime < 200)
		{
			AttackInfo& atk = it->second;
			DrawAttackEffect(pDC, atk.startPos, atk.dir);
			++it;
		}
		else
		{
			it = m_otherAttacks.erase(it);
		}
	}
}

void CTestView::DrawAttackEffect(CDC* pDC, CPoint startPos, Direction attackDir)
{
	CPen attackPen(PS_SOLID, 2, RGB(255, 0, 0));
	CPen* pOldPen = pDC->SelectObject(&attackPen);

	CPoint start = startPos;
	CPoint end = start;

	const int offset = 20;
	const int length1 = 10;
	const int length2 = 30;

	switch (attackDir)
	{
	case Direction::Left:
		start.x -= offset;
		start.y += length2 / 2;
		end.x = start.x - length1;
		end.y = start.y - length2;
		break;
	case Direction::Right:
		start.x += offset;
		start.y -= length2 / 2;
		end.x = start.x + length1;
		end.y = start.y + length2;
		break;
	case Direction::Up:
		start.y -= offset;
		start.x -= length2 / 2;
		end.y = start.y - length1;
		end.x = start.x + length2;
		break;
	case Direction::Down:
		start.y += offset;
		start.x += length2 / 2;
		end.y = start.y + length1;
		end.x = start.x - length2;
		break;
	}

	pDC->MoveTo(start);
	pDC->LineTo(end);
	pDC->SelectObject(pOldPen);
}

void CTestView::UpdatePlayer(float deltaTime)
{
	float speed = 150.0f;

	if (m_isMoving)
	{
		float distance = speed * deltaTime;

		CPoint beforePos = m_playerPos;

		switch (m_playerDirection)
		{
		case Direction::Left:  m_playerPos.x -= distance; break;
		case Direction::Right: m_playerPos.x += distance; break;
		case Direction::Up:    m_playerPos.y -= distance; break;
		case Direction::Down:  m_playerPos.y += distance; break;
		}

		ClampPosition(m_playerPos); // ���� ����

		// ��ġ�� �ٲ���� ��쿡�� ������ �̵� ��Ŷ ����
		if (m_playerPos != beforePos && m_network)
		{
			m_network->PlayerMove(m_playerDirection, m_currentFrameID);

			PlayerInput input{ m_currentFrameID, m_playerDirection };

			// �Է� ���� ����
			m_inputBuffer.push_back(input);

			m_currentFrameID++;
		}
	}
}

void CTestView::UpdateOtherPlayers(float deltaTime)
{
}

void CTestView::ClampPosition(CPoint& pos)
{
	if (pos.x < 10)
		pos.x = 10;
	else if (pos.x > MAP_MAX_X + 10)
		pos.x = MAP_MAX_X + 10;
	if (pos.y < 10)
		pos.y = 10;
	else if (pos.y > MAP_MAX_Y + 10)
		pos.y = MAP_MAX_Y + 10;
}

void CTestView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		auto now = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(now - m_lastUpdateTime).count();

		UpdatePlayer(deltaTime);

		m_lastUpdateTime = now;

		Invalidate(FALSE);
	}

	CView::OnTimer(nIDEvent);
}

LRESULT CTestView::OnUpdateMonsterState(WPARAM wParam, LPARAM lParam)
{
	auto* pMonsters = reinterpret_cast<std::vector<S2CMonsterStateInfo>*>(lParam);
	if (pMonsters)
	{
		for (const auto& info : *pMonsters)
		{
			auto it = m_monsters.find(info.SpawnID);
			if (it != m_monsters.end())
			{
				it->second->ApplyState(info);
			}
			else
			{
				// ���ο� ���� ���� (ó�� ���� ���)
				auto monster = std::make_unique<Monster>();
				monster->SetID(info.MonsterID);
				monster->Respawn(
					Vector3(info.PosX, info.PosY, info.PosZ),
					info.MaxHP, info.CurHP);
				m_monsters[info.SpawnID] = std::move(monster);
			}
		}

		delete pMonsters;
		Invalidate(false);
	}

	return 0;
}

LRESULT CTestView::OnMapChange(WPARAM wParam, LPARAM lParam)
{
	m_playerPos.x = wParam;
	m_playerPos.y = lParam;

	Invalidate(false);

	return 0;
}

LRESULT CTestView::OnPlayerEnter(WPARAM wParam, LPARAM lParam)
{
	auto* pInfo = reinterpret_cast<OtherPlayer*>(lParam);
	if (pInfo && m_user->GetUserId() != pInfo->userID)
	{
		m_otherPlayers.insert({ pInfo->userID, *pInfo });
		delete pInfo;
		Invalidate(false);
	}

	return 0;
}

LRESULT CTestView::OnPlayerLeave(WPARAM wParam, LPARAM lParam)
{
	uint16_t userID = static_cast<uint16_t>(wParam);

	m_otherPlayers.erase(userID);

	Invalidate(false);
	return 0;
}

LRESULT CTestView::OnUpdatePlayerState(WPARAM wParam, LPARAM lParam)
{
	auto* pInfo = reinterpret_cast<std::vector<S2CPlayerStateInfo>*>(lParam);

	if (pInfo)
	{
		m_otherPlayers.clear();

		for (const auto& info : *pInfo)
		{
			OtherPlayer player;
			player.userID = info.UserID;
			CA2W nameConverter(info.Name);
			player.name = nameConverter;

			player.position.x = info.PosX;
			player.position.y = info.PosY;
			player.userID = info.UserID;

			if (player.userID != m_user->GetUserId())
			{
				m_otherPlayers.insert({ player.userID, player });
			}
		}
		delete pInfo;
		Invalidate(false);
	}
	return 0;
}

LRESULT CTestView::OnOtherPlayerMove(WPARAM wParam, LPARAM lParam)
{
	uint16_t userID = static_cast<uint16_t>(wParam);
	Direction direction = static_cast<Direction>(lParam);

	auto it = m_otherPlayers.find(userID);
	if (it != m_otherPlayers.end())
	{
		switch (direction)
		{
		case Direction::Left:
			it->second.position.x -= 10;
			break;
		case Direction::Right:
			it->second.position.x += 10;
			break;
		case Direction::Up:
			it->second.position.y -= 10;
			break;
		case Direction::Down:
			it->second.position.y += 10;
			break;
		default:
			break;
		}
	}
	Invalidate(false);

	return 0;
}

LRESULT CTestView::OnPlayerPosSync(WPARAM wParam, LPARAM lParam)
{
	uint32_t ackFrame = static_cast<uint32_t>(wParam);
	Vector3 serverPos = *reinterpret_cast<Vector3*>(&lParam);
	CPoint serverPosPoint(static_cast<int>(serverPos.x), static_cast<int>(serverPos.y));

	const float ALLOWED_ERROR = 5.0f;

	float dx = m_playerPos.x - serverPosPoint.x;
	float dy = m_playerPos.y - serverPosPoint.y;
	float distance = sqrtf(dx * dx + dy * dy);

	if (distance > ALLOWED_ERROR)
	{
		// ���� ������� ��ġ ����
		const float correctionRate = 0.2f;
		m_playerPos.x += (serverPosPoint.x - m_playerPos.x) * correctionRate;
		m_playerPos.y += (serverPosPoint.y - m_playerPos.y) * correctionRate;

		// ���� �Է� ����
		while (!m_inputBuffer.empty() && m_inputBuffer.front().frameID <= ackFrame)
		{
			m_inputBuffer.pop_front();
		}

		// ���� �Է� ������
		const float fixedDelta = 0.1f;
		float speed = 150.0f;

		for (const auto& input : m_inputBuffer)
		{
			float distance = speed * fixedDelta;

			switch (input.dir)
			{
			case Direction::Left:  m_playerPos.x -= distance; break;
			case Direction::Right: m_playerPos.x += distance; break;
			case Direction::Up:    m_playerPos.y -= distance; break;
			case Direction::Down:  m_playerPos.y += distance; break;
			}

			ClampPosition(m_playerPos);
		}

		Invalidate(FALSE);
	}

	return 0;
}

LRESULT CTestView::OnUpdateMonsterHit(WPARAM wParam, LPARAM lParam)
{
	auto* pHitInfo = reinterpret_cast<std::vector<S2CMonsterHitInfo>*>(lParam);
	if (pHitInfo)
	{
		for (const auto& hitInfo : *pHitInfo)
		{
			auto it = m_monsters.find(hitInfo.SpawnID);
			if (it != m_monsters.end())
			{
				it->second->ApplyHit(hitInfo);
				if (it->second->IsDead())
				{
					// ���Ͱ� �׾��� �� �߰� ó�� (��: ����, �ִϸ��̼� ��)
					m_monsters.erase(it); // ���� ����
				}
			}
		}

		delete pHitInfo;
		Invalidate(false);
	}
	return 0;
}

LRESULT CTestView::OnRespawnMonster(WPARAM wParam, LPARAM lParam)
{
	auto* pRespawnMonsters = reinterpret_cast<std::vector<S2CMonsterRespawnInfo>*>(lParam);
	if (pRespawnMonsters)
	{
		for (const auto& info : *pRespawnMonsters)
		{
			auto it = m_monsters.find(info.SpawnID);
			if (it != m_monsters.end())
			{
				// �̹� �ִ� ���Ͱ� ��������� ��ŵ
				if (!it->second->IsDead())
					continue;

				// �׾��ִ� ���͸� ������
				it->second->Respawn(
					Vector3(info.SpawnPosX, info.SpawnPosY, info.SpawnPosZ),
					info.MaxHP, info.CurHP);
			}
			else
			{
				// ���ο� ���� ���� (ó�� ���� ���)
				auto monster = std::make_unique<Monster>();
				monster->SetID(info.MonsterID);
				monster->Respawn(
					Vector3(info.SpawnPosX, info.SpawnPosY, info.SpawnPosZ),
					info.MaxHP, info.CurHP);
				m_monsters[info.SpawnID] = std::move(monster);
			}
		}

		delete pRespawnMonsters;
		Invalidate(false);
	}

	return 0;
}

LRESULT CTestView::OnPlayerAttack(WPARAM wParam, LPARAM lParam)
{
	uint16_t userID = static_cast<uint16_t>(wParam);
	Direction dir = static_cast<Direction>(lParam);

	auto it = m_otherPlayers.find(userID);
	if (it != m_otherPlayers.end())
	{
		CPoint pos;
		pos.x = it->second.position.x;
		pos.y = it->second.position.y;

		AttackInfo info;
		info.startTime = GetTickCount64();
		info.startPos = pos;
		info.dir = dir;

		m_otherAttacks[userID] = info;
		Invalidate(false);
	}

	return 0;
}

BOOL CTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	return CView::PreCreateWindow(cs);
}