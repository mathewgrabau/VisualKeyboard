#include <afxwin.h>
#include "VisualKeyboard.h"

BOOL CVisualKeyboardApp::InitInstance()
{
	m_pMainWnd = new CMainWindow();
	m_pMainWnd->ShowWindow(m_nCmdShow);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}

BEGIN_MESSAGE_MAP(CMainWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_CHAR()
	ON_WM_SYSCHAR()
END_MESSAGE_MAP()

CMainWindow::CMainWindow()
{
	m_nTextPos = 0;
	m_nMsgPos = 0;

	// Load cursors
	m_hCursorArrow = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	m_hCursorIBeam = AfxGetApp()->LoadStandardCursor(IDC_IBEAM);

	// Register the window class
	CString strWndClass = AfxRegisterWndClass(0, NULL, (HBRUSH)(COLOR_3DFACE + 1), AfxGetApp()->LoadStandardIcon(IDI_WINLOGO));

	// Create a window
	CreateEx(0, strWndClass, _T("Visual Keyboard"), WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL);
}

int CMainWindow::GetNearestPos(CPoint point)
{
	// Return 0 if (point.x, point.y) lies to the lies to the left of the text in the text box.
	if (point.x < m_ptTextOrigin.x)
	{
		return 0;
	}

	// Return the string length if (point.x, point.y) lies to the right of the text in the text box.
	CClientDC dc(this);
	int nLen = m_strInputText.GetLength();
	if (point.x >= (m_ptTextOrigin.x + (dc.GetTextExtent(m_strInputText, nLen)).cx))
	{
		return nLen;
	}

	// Knowing that (point.x, point.y) lies somewhere within the text in the text box, convert the coordinates into a character index.
	int i = 0;
	int nPrevChar = m_ptTextOrigin.x;
	int nNextChar = m_ptTextOrigin.x;

	while (nNextChar < point.x)
	{
		i++;
		nPrevChar = nNextChar;
		nNextChar = m_ptTextOrigin.x + (dc.GetTextExtent(m_strInputText.Left(i), i)).cx;
	}

	return ((point.x - nPrevChar) < (nNextChar - point.x)) ? i - 1 : i;

	return 0;
}

void CMainWindow::PositionCaret(CDC* pDC)
{
}

void CMainWindow::DrawInputText(CDC* pDC)
{
	pDC->ExtTextOut(m_ptTextOrigin.x, m_ptTextOrigin.y, ETO_OPAQUE, m_rcTextBox, m_strInputText, NULL);
}

void CMainWindow::ShowMessage(LPCTSTR pszMessage, UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Formulate a message string
	CString string;
	string.Format(_T("%s\t %u\t %u\t %u\t %u\t %u\t %u\t %u"),
		pszMessage, nChar, nRepCnt, nFlags & 0xFF, (nFlags >> 8) & 0x1, (nFlags >> 13) & 0x1, (nFlags >> 14) & 0x1, (nFlags >> 15) & 0x1);

	// Scroll the other message strings up and validate the scroll rectangle to prevent OnPaint from being called.
	ScrollWindow(0, -m_cyLine, &m_rcScroll);
	ValidateRect(m_rcScroll);

	// Record the new message string and display it in the window.
	CClientDC dc(this);
	dc.SetBkColor((COLORREF)::GetSysColor(COLOR_3DFACE));

	m_strMessages[m_nMsgPos] = string;
	dc.TabbedTextOut(m_ptLowerMsgOrigin.x, m_ptLowerMsgOrigin.y, m_strMessages[m_nMsgPos], m_strMessages[m_nMsgPos].GetLength(), sizeof(m_nTabStops), m_nTabStops, m_ptLowerMsgOrigin.x);

	// Update the array index that specifies where the next message string will be stored.
	if (++m_nMsgPos == MAX_STRINGS)
	{
		m_nMsgPos = 0;
	}
}



int CMainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// NNeed to initialize a bunch of this.
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	// Text metric related variables
	CClientDC dc(this);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	m_cxChar = tm.tmAveCharWidth;
	m_cyChar = tm.tmHeight;
	m_cyLine = tm.tmHeight + tm.tmExternalLeading;

#define WINDOW_PADDING 16

	m_rcTextBoxBorder.SetRect(WINDOW_PADDING, WINDOW_PADDING, (m_cxChar * 64) + 16, (m_cyChar * 3) / 2 + WINDOW_PADDING);

	// Size the text area based on the border (put it in side there)
	m_rcTextBox = m_rcTextBoxBorder;
	m_rcTextBox.InflateRect(-2, -2);

	m_rcMsgBoxBorder.SetRect(WINDOW_PADDING, (m_cyChar * 4) + 16, (m_cxChar * 64) + WINDOW_PADDING, (m_cyLine + MAX_STRINGS) + (m_cyChar * 6) + 16);
	
	m_rcScroll.SetRect(m_cxChar + WINDOW_PADDING, (m_cyChar * 6) + 16, (m_cxChar * 64) + 16, (m_cyLine * MAX_STRINGS) + (m_cyChar * 5) + WINDOW_PADDING);
	
	m_ptTextOrigin.x = m_cxChar + WINDOW_PADDING;
	m_ptTextOrigin.y = m_cyChar + WINDOW_PADDING;
	m_ptCaretPos = m_ptTextOrigin;
	m_nTextLimit = (m_cxChar * 63) + WINDOW_PADDING;

	m_ptHeaderOrigin.x = m_cxChar + WINDOW_PADDING;
	m_ptHeaderOrigin.y = (m_cyChar * 3) + WINDOW_PADDING;

	m_ptUpperMsgOrigin.x = m_cxChar + WINDOW_PADDING;
	m_ptUpperMsgOrigin.x = (m_cyChar * 5) + WINDOW_PADDING;

	m_ptLowerMsgOrigin.x = m_cxChar + WINDOW_PADDING;
	m_ptLowerMsgOrigin.y = (m_cyChar * 5) + (m_cyLine * (MAX_STRINGS - 1)) + WINDOW_PADDING;

	for (int i = 0; i < _countof(m_nTabStops); ++i)
	{
		m_nTabStops[i] = (m_cxChar * (24 + 6 * i)) + WINDOW_PADDING;
	}

	// Sizing the window
	CRect rect(0, 0, m_rcMsgBoxBorder.right + WINDOW_PADDING, m_rcMsgBoxBorder.bottom + WINDOW_PADDING);
	CalcWindowRect(&rect);

	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);

	return 0;
}

void CMainWindow::PostNcDestroy()
{
	delete this;
}

void CMainWindow::OnPaint()
{
	CPaintDC dc(this);

	// Draw boxes around the areas where text is going
	dc.DrawEdge(m_rcTextBoxBorder, EDGE_SUNKEN, BF_RECT);
	dc.DrawEdge(m_rcMsgBoxBorder, EDGE_SUNKEN, BF_RECT);

	// Draw the text
	DrawInputText(&dc);
	DrawMessageHeader(&dc);
	DrawMessages(&dc);
}

void CMainWindow::OnSetFocus(CWnd* pWnd)
{
	// Show caret
	CreateSolidCaret(max(2, ::GetSystemMetrics(SM_CXBORDER)), m_cyChar);
	SetCaretPos(m_ptCaretPos);
	ShowCaret();
}

void CMainWindow::OnKillFocus(CWnd* pwnd)
{
	HideCaret();
	m_ptCaretPos = GetCaretPos();
	::DestroyCaret();
}

BOOL CMainWindow::OnSetCursor(CWnd* pwnd, UINT nHitTest, UINT message)
{
	// Change cursor to the caret if currently over the the text box, arrow otherwise.
	if (nHitTest == HTCLIENT)
	{
		DWORD dwPos = ::GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		ScreenToClient(&point);
		::SetCursor(m_rcTextBox.PtInRect(point) ? m_hCursorIBeam : m_hCursorArrow);
	}

	return CWnd::OnSetCursor(pwnd, nHitTest, message);
}

void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Move the caret if the text box is clicked with the left mouse button
	if (m_rcTextBox.PtInRect(point))
	{
		m_nTextPos = GetNearestPos(point);
		PositionCaret();
	}
}

void CMainWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMainWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMainWindow::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMainWindow::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMainWindow::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMainWindow::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMainWindow::DrawMessageHeader(CDC* pDC)
{
	static CString string = _T("Message\tChar\tRep\tScan\tExt\tCon\tPrv\tTran");

	pDC->SetBkColor((COLORREF)::GetSysColor(COLOR_3DFACE));
	pDC->TabbedTextOut(m_ptHeaderOrigin.x, m_ptHeaderOrigin.y, string, string.GetLength(), sizeof(m_nTabStops), m_nTabStops, m_ptHeaderOrigin.x);
}

void CMainWindow::DrawMessages(CDC* pDC)
{
	// This method draws the strings on the function.
	int nPos = m_nMsgPos;
	pDC->SetBkColor((COLORREF)::GetSysColor(COLOR_3DFACE));

	for (int i = 0; i < MAX_STRINGS; ++i)
	{
		pDC->TabbedTextOut(m_ptUpperMsgOrigin.x, m_ptUpperMsgOrigin.y + (m_cyLine * i), m_strMessages[nPos], m_strMessages[nPos].GetLength(), sizeof(m_nTabStops), m_nTabStops, m_ptUpperMsgOrigin.x);

		if (++nPos == MAX_STRINGS)
		{
			nPos = 0;
		}
	}
}
