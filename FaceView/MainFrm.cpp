
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "FaceView.h"
#include "FaceViewDoc.h"
#include "FaceViewView.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_RECOGNISER, &CMainFrame::OnUpdateRecogniser)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_GENDER, &CMainFrame::OnUpdateGender)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_VISEME,
	ID_INDICATOR_RECOGNISER,
	ID_INDICATOR_GENDER,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_wndSplitterInit=0;
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

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	m_wndSplitter.CreateStatic( this, 1, 2);
	m_wndSplitter.CreateView(0,0,RUNTIME_CLASS(CFaceViewView),CSize(50,50),pContext);
	m_wndSplitter.CreateView(0,1,RUNTIME_CLASS(CFaceViewConsole),CSize(50,50),pContext);

	CRect cr;
	GetClientRect(&cr);
	TRACE("Client=%dx%d\n",cr.Width(),cr.Height());

	m_wndSplitter.SetRowInfo(0,cr.Height(),50);
	m_wndSplitter.SetColumnInfo(0,cr.Width()/2,50);
	m_wndSplitter.SetColumnInfo(1,cr.Width()/2,50);

	m_wndSplitterInit=1;

	m_wndSplitter.SetActivePane(0,0);

	return TRUE;
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


void CMainFrame::Console(LPCTSTR str)
{
	if (m_wndSplitterInit) {
		CFaceViewConsole *pView = (CFaceViewConsole *)m_wndSplitter.GetPane(0,1);
		if (pView) pView->AddString(str);
	}
}


CFaceViewConsole * CMainFrame::GetConsoleView(void)
{
	if (m_wndSplitterInit)
		return (CFaceViewConsole *)m_wndSplitter.GetPane(0,1);
	else
		return NULL;
}

void CMainFrame::OnUpdateRecogniser(CCmdUI *pCmdUI)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	pCmdUI->Enable();
	CString txt="";
	if (pApp->m_recogniser==0) txt="AnnoSoft";
	pCmdUI->SetText(txt);
}

void CMainFrame::OnUpdateGender(CCmdUI *pCmdUI)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	pCmdUI->Enable();
	CString txt="";
	if (pApp->m_gender==0) 
		txt="Male";
	else if (pApp->m_gender==1) 
		txt="Female";
	else
		txt="Generic";
	pCmdUI->SetText(txt);
}
