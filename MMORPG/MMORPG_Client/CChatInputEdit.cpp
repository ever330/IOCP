#include "pch.h"
#include "CChatInputEdit.h"
#include "Resource.h"

BEGIN_MESSAGE_MAP(CChatInputEdit, CEdit)
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()

void CChatInputEdit::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bAllowFocus = TRUE;
	CEdit::OnLButtonDown(nFlags, point);
}

void CChatInputEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_RETURN)
	{
		if (m_pParent)
			m_pParent->SendMessage(WM_COMMAND, MAKEWPARAM(IDC_SEND_BUTTON, BN_CLICKED));

		return;
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

UINT CChatInputEdit::OnGetDlgCode()
{
	return DLGC_WANTARROWS | CEdit::OnGetDlgCode();
}