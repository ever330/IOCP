#include "CTestView.h"
#include "Network.h"
#include "Vector3.h"

BEGIN_MESSAGE_MAP(CTestView, CWnd)
	ON_WM_GETDLGCODE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_UPDATE_MONSTER_STATE, &CTestView::OnUpdateMonsterState)
	ON_MESSAGE(WM_MAP_CHANGE, &CTestView::OnMapChange)
	ON_MESSAGE(WM_PLAYER_ENTER, &CTestView::OnPlayerEnter)
	ON_MESSAGE(WM_PLAYER_LEAVE, &CTestView::OnPlayerLeave)
	ON_MESSAGE(WM_UPDATE_PLAYER_STATE, &CTestView::OnUpdatePlayerState)
	ON_MESSAGE(WM_OTHER_PLAYER_POS_SYNC, &CTestView::OnOtherPlayerPosSync)
	ON_MESSAGE(WM_PLAYER_POS_SYNC, &CTestView::OnPlayerPosSync)
	ON_MESSAGE(WM_UPDATE_MONSTER_HIT, &CTestView::OnUpdateMonsterHit)
	ON_MESSAGE(WM_MONSTER_RESPAWN, &CTestView::OnRespawnMonster)
	ON_MESSAGE(WM_PLAYER_ATTACK, &CTestView::OnPlayerAttack)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CTestView::CTestView() 
{
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
	m_syncTimer = 0.0f;
}

CTestView::~CTestView()
{
}

BOOL CTestView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;  // 깜빡임 방지 효과
}

void CTestView::OnPaint() 
{
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap);
	DrawScene(&memDC);
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);
}

void CTestView::Initialize() 
{
	UINT_PTR timerID = SetTimer(1, 100, nullptr);
	if (timerID == 0) {
		AfxMessageBox(_T("타이머 설정 실패!"));
	}
	m_lastUpdateTime = std::chrono::steady_clock::now();
}

void CTestView::SetNetwork(Network* network) 
{
	m_network = network;
}

void CTestView::SetUser(User* user) 
{
	m_user = user;
}

void CTestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	DWORD now = GetTickCount64();

	// 스페이스바(공격)는 이동 판단 없이 따로 처리
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
		return; // 여기서 끝냄 (아래 이동 로직 실행 X)
	}
	else if (nChar == VK_UP)
	{
		for (auto& portal : m_portals)
		{
			Vector3 pos = Vector3(m_playerPos.x, m_playerPos.y, 0.0f);
			if (portal.second->OnEnter(pos))
			{
				if (m_network)
				{
					m_network->ChangeMapByPortal(portal.first);
				}
				return; // 포탈 클릭 시 이동 로직 실행 X
			}
		}
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

	// 서버 전송
	if (m_network)
	{
		uint32_t frameID = m_currentFrameID++;
		m_inputBuffer.push_back({ frameID, m_playerDirection });
		m_network->PlayerMove(m_playerDirection, frameID);
	}

	Invalidate(FALSE);
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
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

	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CTestView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();
	CWnd::OnLButtonDown(nFlags, point);
}

UINT CTestView::OnGetDlgCode()
{
	return DLGC_WANTARROWS | CWnd::OnGetDlgCode();
}

void CTestView::DrawScene(CDC* pDC)
{
	CRect rect;
	GetClientRect(&rect);

	// 배경
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	// 포탈
	for (auto& portal : m_portals)
	{
		portal.second->Render(pDC);
	}

	// 몬스터
	for (auto& monster : m_monsters)
	{
		monster.second->Render(pDC);
	}

	// 다른 플레이어들
	for (const auto& player : m_otherPlayers)
	{
		player.second->Render(pDC);
	}

	// 내 플레이어
	pDC->FillSolidRect(m_playerPos.x - 10, m_playerPos.y - 10, 20, 20, RGB(0, 0, 255));

	DWORD now = GetTickCount64();
	// 공격 모션
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

	// 다른 플레이어의 공격 모션
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

		ClampPosition(m_playerPos); // 범위 제한

		// 위치가 바뀌었을 경우에만 서버로 이동 패킷 전송
		if (m_playerPos != beforePos && m_network)
		{
			m_network->PlayerMove(m_playerDirection, m_currentFrameID);

			PlayerInput input{ m_currentFrameID, m_playerDirection };

			// 입력 버퍼 저장
			m_inputBuffer.push_back(input);

			m_currentFrameID++;
		}
	}
}

void CTestView::UpdateOtherPlayers(float deltaTime)
{
	for (auto& player : m_otherPlayers)
	{
		player.second->Update(deltaTime);
	}
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
		UpdateOtherPlayers(deltaTime);
		m_lastUpdateTime = now;

		m_syncTimer += deltaTime;

		if (m_syncTimer >= 0.2f)
		{
			m_network->PlayerPosSync(m_playerPos.x, m_playerPos.y, 0.0f, m_currentFrameID);
			m_syncTimer = 0.0f;
		}

		Invalidate(FALSE);
	}
	CWnd::OnTimer(nIDEvent);
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
				// 새로운 몬스터 생성 (처음 들어온 경우)
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
	PacketBase* pac = reinterpret_cast<PacketBase*>(lParam);
	S2CChangeMapAckPacket* changeMap = reinterpret_cast<S2CChangeMapAckPacket*>(pac->Body);

	std::vector<S2CChangeMapPortalInfo> portals(changeMap->PortalCount);
	memcpy(portals.data(), pac->Body + sizeof(S2CChangeMapAckPacket), sizeof(S2CChangeMapPortalInfo) * changeMap->PortalCount);
	m_portals.clear();

	for (const auto& portal : portals)
	{
		auto info = std::make_unique<Portal>(
			static_cast<unsigned int>(portal.PortalID),
			Vector3(portal.PosX, portal.PosY, portal.PosZ),
			portal.TargetMapID
		);
		m_portals[portal.PortalID] = std::move(info);
	}

	m_playerPos.x = changeMap->SpawnPosX;
	m_playerPos.y = changeMap->SpawnPosY;

	Invalidate(false);

	return 0;
}

LRESULT CTestView::OnPlayerEnter(WPARAM wParam, LPARAM lParam)
{
	auto rawPtr = reinterpret_cast<OtherPlayer*>(lParam);
	std::unique_ptr<OtherPlayer> otherPlayer(rawPtr);

	if (otherPlayer && m_user->GetActiveCharacter().ID != otherPlayer->GetID())
	{
		m_otherPlayers[otherPlayer->GetID()] = std::move(otherPlayer);
		Invalidate(false);
	}

	return 0;
}

LRESULT CTestView::OnPlayerLeave(WPARAM wParam, LPARAM lParam)
{
	uint16_t characterID = static_cast<uint16_t>(wParam);

	m_otherPlayers.erase(characterID);

	Invalidate(false);
	return 0;
}

LRESULT CTestView::OnUpdatePlayerState(WPARAM wParam, LPARAM lParam)
{
	auto* pInfo = reinterpret_cast<std::vector<S2CPlayerStateInfo>*>(lParam);

	if (pInfo)
	{
		for (const auto& info : *pInfo)
		{
			auto player = m_otherPlayers.find(info.CharacterID);
			if (player != m_otherPlayers.end())
			{
				// 이미 있는 플레이어의 상태 업데이트
				player->second->SetTargetPosition(info.PosX, info.PosY);
				player->second->SetDirection(static_cast<Direction>(info.Direction));
				continue;
			}
			else
			{
				// 새로운 플레이어 생성
				if (info.CharacterID == m_user->GetActiveCharacter().ID)
					continue; // 자신의 캐릭터는 스킵
				auto newPlayer = std::make_unique<OtherPlayer>(info.CharacterID, CString(info.Name), Vector3(info.PosX, info.PosY, 0.0f));
				newPlayer->SetDirection(static_cast<Direction>(info.Direction));
				m_otherPlayers[info.CharacterID] = std::move(newPlayer);
			}
		}
		delete pInfo;
		Invalidate(false);
	}
	return 0;
}

LRESULT CTestView::OnOtherPlayerPosSync(WPARAM wParam, LPARAM lParam)
{
	auto* sync = reinterpret_cast<S2COtherPlayerPosSyncPacket*>(lParam);

	auto it = m_otherPlayers.find(sync->CharacterID);
	if (it != m_otherPlayers.end())
	{
		it->second->SetTargetPosition(sync->PosX, sync->PosY);
		it->second->SetDirection(static_cast<Direction>(sync->MoveDirection));
	}

	return 0;
}

LRESULT CTestView::OnPlayerPosSync(WPARAM wParam, LPARAM lParam)
{
	//uint32_t ackFrame = static_cast<uint32_t>(wParam);
	//Vector3 serverPos = *reinterpret_cast<Vector3*>(&lParam);
	//CPoint serverPosPoint(static_cast<int>(serverPos.x), static_cast<int>(serverPos.y));

	//const float ALLOWED_ERROR = 5.0f;

	//float dx = m_playerPos.x - serverPosPoint.x;
	//float dy = m_playerPos.y - serverPosPoint.y;
	//float distance = sqrtf(dx * dx + dy * dy);

	//if (distance > ALLOWED_ERROR)
	//{
	//	// 보간 방식으로 위치 보정
	//	const float correctionRate = 0.2f;
	//	m_playerPos.x += (serverPosPoint.x - m_playerPos.x) * correctionRate;
	//	m_playerPos.y += (serverPosPoint.y - m_playerPos.y) * correctionRate;

	//	// 이전 입력 제거
	//	while (!m_inputBuffer.empty() && m_inputBuffer.front().frameID <= ackFrame)
	//	{
	//		m_inputBuffer.pop_front();
	//	}

	//	// 남은 입력 재적용
	//	const float fixedDelta = 0.1f;
	//	float speed = 150.0f;

	//	for (const auto& input : m_inputBuffer)
	//	{
	//		float distance = speed * fixedDelta;

	//		switch (input.dir)
	//		{
	//		case Direction::Left:  m_playerPos.x -= distance; break;
	//		case Direction::Right: m_playerPos.x += distance; break;
	//		case Direction::Up:    m_playerPos.y -= distance; break;
	//		case Direction::Down:  m_playerPos.y += distance; break;
	//		}

	//		ClampPosition(m_playerPos);
	//	}

	//	Invalidate(FALSE);
	//}

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
					// 몬스터가 죽었을 때 추가 처리 (예: 제거, 애니메이션 등)
					m_monsters.erase(it); // 몬스터 제거
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
				// 이미 있는 몬스터가 살아있으면 스킵
				if (!it->second->IsDead())
					continue;

				// 죽어있는 몬스터만 리스폰
				it->second->Respawn(
					Vector3(info.SpawnPosX, info.SpawnPosY, info.SpawnPosZ),
					info.MaxHP, info.CurHP);
			}
			else
			{
				// 새로운 몬스터 생성 (처음 들어온 경우)
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
	uint16_t characterID = static_cast<uint16_t>(wParam);
	Direction dir = static_cast<Direction>(lParam);

	auto it = m_otherPlayers.find(characterID);
	if (it != m_otherPlayers.end())
	{
		Vector3 p = it->second->GetPosition();
		CPoint pos = CPoint(p.x, p.y);

		AttackInfo info;
		info.startTime = GetTickCount64();
		info.startPos = pos;
		info.dir = dir;

		m_otherAttacks[characterID] = info;
		Invalidate(false);
	}

	return 0;
}

BOOL CTestView::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	return CWnd::PreCreateWindow(cs);
}