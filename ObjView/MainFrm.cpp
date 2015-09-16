// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ObjView.h"
#include "ObjViewDoc.h"
#include "ObjViewView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ROTX, OnUpdateRotX)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ROTY, OnUpdateRotY)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_ROTZ, OnUpdateRotZ)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_ROTX,
	ID_INDICATOR_ROTY,
	ID_INDICATOR_ROTZ,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers



// update the status bar
void CMainFrame::OnUpdateRotX(CCmdUI *pCmdUI)
{
	CObjViewDoc *pDoc = static_cast<CObjViewDoc *> (GetActiveView()->GetDocument());
	CString msg;
	msg.Format("%03g",pDoc->m_rotx);
	pCmdUI->Enable();
	pCmdUI->SetText((LPCTSTR)msg);
}
void CMainFrame::OnUpdateRotY(CCmdUI *pCmdUI)
{
	CObjViewDoc *pDoc = static_cast<CObjViewDoc *> (GetActiveView()->GetDocument());
	CString msg;
	msg.Format("%03g",pDoc->m_roty);
	pCmdUI->Enable();
	pCmdUI->SetText((LPCTSTR)msg);
}
void CMainFrame::OnUpdateRotZ(CCmdUI *pCmdUI)
{
	CObjViewDoc *pDoc = static_cast<CObjViewDoc *> (GetActiveView()->GetDocument());
	CString msg;
	msg.Format("%03g",pDoc->m_rotz);
	pCmdUI->Enable();
	pCmdUI->SetText((LPCTSTR)msg);
}
