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
	ON_MESSAGE(WM_UPDATE_MONSTER_HIT, &CTestView::OnUpdateMonsterHit)
	ON_MESSAGE(WM_MONSTER_RESPAWN, &CTestView::OnRespawnMonster)
	ON_MESSAGE(WM_PLAYER_ATTACK, &CTestView::OnPlayerAttack)
	ON_WM_KEYDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()

CTestView::CTestView()
{
	// 초기 위치 설정
	m_playerPos = CPoint(100, 100);
	m_network = nullptr;
}

CTestView::~CTestView()
{
}

BOOL CTestView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;  // 깜빡임 방지 효과
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

	// 실제 게임 씬을 메모리 DC에 그리기
	DrawScene(&memDC);

	// 최종적으로 화면에 복사
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBitmap);
}

void CTestView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	SetTimer(1, 16, nullptr); // 약 60 FPS
	m_prevTime = GetTickCount64();
}

void CTestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	DWORD now = GetTickCount64();

	switch (nChar)
	{
	case VK_LEFT:
		m_playerPos.x -= 10;
		if (m_playerPos.x < 10)
		{
			m_playerPos.x = 10;
			break;
		}

		if (m_network)
			m_network->PlayerMove(Direction::Left);

		m_playerDirection = Direction::Left;
		break;

	case VK_RIGHT:
		m_playerPos.x += 10;
		if (m_playerPos.x > MAP_MAX_X + 10) // 화면 너비 - 플레이어 크기
		{
			m_playerPos.x = MAP_MAX_X + 10;
			break;
		}

		if (m_network)
			m_network->PlayerMove(Direction::Right);

		m_playerDirection = Direction::Right;
		break;

	case VK_UP:
		m_playerPos.y -= 10;
		if (m_playerPos.y < 10)
		{
			m_playerPos.y = 10;
			break;
		}

		if (m_network)
			m_network->PlayerMove(Direction::Up);

		m_playerDirection = Direction::Up;
		break;

	case VK_DOWN:
		m_playerPos.y += 10;
		if (m_playerPos.y > MAP_MAX_Y + 10)
		{
			m_playerPos.y = MAP_MAX_Y + 10;
			break;
		}

		if (m_network)
			m_network->PlayerMove(Direction::Down);

		m_playerDirection = Direction::Down;
		break;

	case VK_SPACE:
		if (now - m_lastAttackTime > 500) // 0.5초 쿨타임
		{
			m_lastAttackTime = now;

			m_isAttacking = true;
			m_attackStartTime = now;

			m_attackStartPos = m_playerPos;
			m_attackDirection = m_playerDirection;

			if (m_network)
				m_network->PlayerAttack(m_playerDirection);
		}
		break;

	default:
		break;
	}

 	Invalidate(false);
	CView::OnKeyDown(nChar, nRepCnt, nFlags); // 기본 처리
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

	// 배경
	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	// 몬스터
	for (auto& monster : m_monsters)
	{
		monster.second->Render(pDC);
	}

	// 다른 플레이어들
	for (const auto& player : m_otherPlayers)
	{
		int x = static_cast<int>(player.second.position.x);
		int y = static_cast<int>(player.second.position.y);

		// 글자 크기 측정
		CString name = player.second.name;
		CSize textSize = pDC->GetTextExtent(name);

		// 배경 사각형 출력
		CRect bgRect(x - 10, y - 25, x - 10 + textSize.cx, y - 25 + textSize.cy);
		pDC->FillSolidRect(bgRect, RGB(255, 255, 255));

		// 텍스트는 투명 배경으로 출력
		pDC->SetBkMode(TRANSPARENT);
		pDC->TextOut(x - 10, y - 25, name);
		pDC->FillSolidRect(x - 10, y - 10, 20, 20, RGB(0, 200, 0));
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

void CTestView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		DWORD currentTime = GetTickCount64();
		float deltaTime = (currentTime - m_prevTime) / 1000.0f; // 초 단위로 변환

		m_prevTime = currentTime;

		// 몬스터 업데이트
		for (auto& monster : m_monsters)
		{
			monster.second->Update(deltaTime);
		}

		Invalidate(false);
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
	m_playerPos.x = wParam;
	m_playerPos.y = lParam;

	Invalidate(false);

	return 0;
}

LRESULT CTestView::OnPlayerEnter(WPARAM wParam, LPARAM lParam)
{
	auto* pInfo = reinterpret_cast<OtherPlayer*>(lParam);
	if (pInfo && m_userNickname != pInfo->name)
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

			if (nameConverter != m_userNickname)
			{
				m_otherPlayers.insert({ player.userID, player });
			}
		}
		delete pInfo;
		Invalidate(false);
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