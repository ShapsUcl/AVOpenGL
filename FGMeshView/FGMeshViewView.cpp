
// FGMeshViewView.cpp : implementation of the CFGMeshViewView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FGMeshView.h"
#endif
#include "MainFrm.h"
#include "FGMeshViewDoc.h"
#include "FGMeshViewView.h"

#include "Fg3dOpenGL.hpp"
#include "FgOpenGL.hpp"
#include "Fg3dOpenGLImpl.hpp"
#include "FgDefaultVal.hpp"

#include "FgCmdView.hpp"
#include "FgCommand.hpp"
#include "FgStdString.hpp"
#include "FgFileSystem.hpp"
#include "Fg3dDisplay.hpp"
#include "Fg3dMeshIo.hpp"
#include "Fg3dMeshOps.hpp"
#include "FgSyntax.hpp"
#include "FgImgDisplay.hpp"
#include "FgDraw.hpp"
#include "FgAffineCwC.hpp"
#include "FgBuild.hpp"

//#include "FgGuiApiBase.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFGMeshViewView

IMPLEMENT_DYNCREATE(CFGMeshViewView, CView)

BEGIN_MESSAGE_MAP(CFGMeshViewView, CView)
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_SIZE()
	//	ON_WM_PAINT()
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CFGMeshViewView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CFGMeshViewView construction/destruction

CFGMeshViewView::CFGMeshViewView()
{
	// TODO: add construction code here
	m_init = 0;
}

CFGMeshViewView::~CFGMeshViewView()
{
}

BOOL CFGMeshViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}


using namespace std;
#include "FgTestUtils.hpp"
//static void viewMesh(const FgArgs & args);	// defined in FgCmdView.cpp

// CFGMeshViewView drawing
//FgCmd
//fgCmdViewMesh3D()
//{
//	TRACE("Calling viewmesh command\n");
//	//args.push_back("mesh");
//	return (FgCmd(viewMesh, "mesh", arglist)); 
//}

CFGMeshViewView * GetCFGMeshView()
{
	CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->m_pMainWnd;
	return (CFGMeshViewView *) pFrame->GetSafeHwnd(); 
};

int CFGMeshViewView::InitGui(void) 
{
	CFGMeshViewDoc* pDoc = (CFGMeshViewDoc *) GetDocument();
	TRACE("InitGui: m_init = %d\n", pDoc->m_init);
	if (pDoc->m_init) 
		return -1;
	std::string args;
	std::string trifile = "HeadHires.tri";
	std::string hairtrifile = "HairMidlengthStraightBrown.tri";
	char * arglist = new char[255];
	sprintf(arglist, "mesh HeadHires.tri HeadHires.bmp HairMidlengthStraightBrown.tri HairMidlengthStraightBrown.tga\0");
	args.assign(arglist);
	fgTestCmd(viewMesh, args);
	pDoc->m_init = 1;
	TRACE("InitGui: m_init = %d\n", pDoc->m_init);
	//FgCmd cmd = fgCmdViewMesh3D(arglist);
	//fgCmdViewMesh3D(fgWhiteBreak(args));
	delete [] arglist;
	return (pDoc->m_init);
}

void CFGMeshViewView::OnDraw(CDC* pDC)
{
	CFGMeshViewDoc* pDoc = (CFGMeshViewDoc *) GetDocument();
	TRACE("View OnDraw: m_init = %d\n", pDoc->m_init);
	int rval;

	ASSERT_VALID(pDoc);
	if (!pDoc)	{
		AfxMessageBox((LPCTSTR)"Error: failed to get document");
		return;
	}
	m_init = pDoc->m_init;
	if (!m_init) {
		TRACE("About to Initialise\n");
		rval = InitGui();
		if (rval == 1) {
			TRACE("Initialised\n");
			pDoc->m_init=1;
		} 
		else 
			if (rval == -1) {
				TRACE("Already initialised\n");
			}
	}


	return;
}


int CFGMeshViewView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	TRACE("View OnCreate\n");
	int rval;

	CMainFrame *pFrame = (CMainFrame *) AfxGetApp()->m_pMainWnd;
	HWND hWnd = GetSafeHwnd();
	HDC hDC = ::GetDC(hWnd);

	// find the best pixel format
	//SetDCPixelFormat(hDC, PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);
	// create OpenGL context and make current
	m_hrc = wglCreateContext(hDC);
	wglMakeCurrent(hDC,m_hrc);
	// TODO: add draw code for native data here

	return 0;
}

void CFGMeshViewView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	return;
//	
}

void CFGMeshViewView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	TRACE("View Onsize(%d,%d)\n",cx,cy);

	if ((cx <= 0)|(cy<=0)) return;

	// re-initialise OpenGL view
	//SetGLState(cx,cy);

	// redraw
	Invalidate(FALSE);
}


void CFGMeshViewView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  //wchar_t     wkey = (wchar_t) nChar;

  //for (size_t ii=0; ii<FgGui.keyHandlers.size(); ++ii) {
  //    FgGuiKeyHandle  kh = FgGui.keyHandlers[ii];
  //    if (wkey == wchar_t(kh.key))
  //        kh.handler();
  //}

}

// CFGMeshViewView printing


void CFGMeshViewView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CFGMeshViewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CFGMeshViewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CFGMeshViewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

void CFGMeshViewView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFGMeshViewView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CFGMeshViewView diagnostics

#ifdef _DEBUG
void CFGMeshViewView::AssertValid() const
{
	CView::AssertValid();
}

void CFGMeshViewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFGMeshViewDoc* CFGMeshViewView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFGMeshViewDoc)));
	return (CFGMeshViewDoc*)m_pDocument;
}
#endif //_DEBUG


// CFGMeshViewView message handlers
