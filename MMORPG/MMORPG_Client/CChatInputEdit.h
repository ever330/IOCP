#pragma once

class CChatInputEdit : public CEdit
{
public:
	CWnd* m_pParent;
	BOOL m_bAllowFocus = FALSE;

	void SetParent(CWnd* pParent) { m_pParent = pParent; }

protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode();
	DECLARE_MESSAGE_MAP()
};