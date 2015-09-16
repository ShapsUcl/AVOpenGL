
// FaceViewView.cpp : implementation of the CFaceViewView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FaceView.h"
#endif

#include "FaceViewDoc.h"
#include "FaceViewConsole.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFaceViewView

IMPLEMENT_DYNCREATE(CFaceViewConsole, CView)

BEGIN_MESSAGE_MAP(CFaceViewConsole, CView)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CFaceViewView construction/destruction

CFaceViewConsole::CFaceViewConsole()
{
	m_plist=NULL;
	m_bgbrush.CreateSolidBrush(RGB(0,0,0));
	m_font.CreatePointFont(120,"Courier",0);
	m_maxstring=0;
}

CFaceViewConsole::~CFaceViewConsole()
{
	if (m_plist) {
		delete m_plist;
	}
	m_bgbrush.DeleteObject();
	m_font.DeleteObject();
}

BOOL CFaceViewConsole::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CFaceViewView drawing

void CFaceViewConsole::OnDraw(CDC* /*pDC*/)
{
//	CFaceViewDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);
//	if (!pDoc)
//		return;

	// TODO: add draw code for native data here
	TRACE("OnDraw()\n");
	if (m_plist) {
		m_plist->UpdateWindow();
		CRect cr;
		m_plist->GetWindowRect(&cr);
		TRACE("OnDraw rect=%d x %d\n",cr.Width(),cr.Height());
	}
}


// CFaceViewView diagnostics

#ifdef _DEBUG
void CFaceViewConsole::AssertValid() const
{
	CView::AssertValid();
}

void CFaceViewConsole::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFaceViewDoc* CFaceViewConsole::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceViewDoc)));
	return (CFaceViewDoc*)m_pDocument;
}
#endif //_DEBUG


// CFaceViewView message handlers


void CFaceViewConsole::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CRect	cr;
	GetClientRect(&cr);

	if (m_plist==NULL) {
		m_plist=new CListBox();
		m_plist->Create(WS_CHILD|WS_VISIBLE|WS_HSCROLL|WS_VSCROLL|LBS_NOSEL|LBS_HASSTRINGS|LBS_NOINTEGRALHEIGHT,cr,this,1);

		m_plist->AddString("This is a console window");
		m_plist->AddString("It will be used to display debugging output");
	}
}


void CFaceViewConsole::AddString(LPCTSTR str)
{
	m_plist->AddString(str);
	int nline=m_plist->GetCount();
	m_plist->SetCurSel(nline-1);
	if (strlen(str) > m_maxstring) {
		m_maxstring=strlen(str);
		CDC *pDC = GetDC();
		CSize size;
		CFont* pOldFont = pDC->SelectObject(&m_font);
	    size = pDC->GetTextExtent(str, (int) _tcslen(str));
	    size.cx += 3;
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
		m_plist->SetHorizontalExtent(size.cx);
	}
	m_plist->UpdateWindow();
}


HBRUSH CFaceViewConsole::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CView::OnCtlColor(pDC, pWnd, nCtlColor);

	// Change any attributes of the DC here
	pDC->SetTextColor(RGB(128,255,128));
	pDC->SetBkColor(RGB(0,0,0));
	pDC->SelectObject(&m_font);

	// Return a different brush if the default is not desired
	return (HBRUSH)m_bgbrush.GetSafeHandle();
}


void CFaceViewConsole::OnSize(UINT nType, int cx, int cy)
{
	// resize child listbox
	if (m_plist && IsWindow(m_plist->m_hWnd)) {
		TRACE("Console Onsize(%d,%d)\n",cx,cy);
		m_plist->MoveWindow(0,0,cx,cy,1);
		Invalidate();
	}

	CView::OnSize(nType, cx, cy);

}


BOOL CFaceViewConsole::OnEraseBkgnd(CDC* pDC)
{
	return CView::OnEraseBkgnd(pDC);
}
