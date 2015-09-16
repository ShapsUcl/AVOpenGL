// ObjViewView.cpp : implementation of the CObjViewView class
//

#include "stdafx.h"
#include "ObjView.h"

#include "ObjViewDoc.h"
#include "ObjViewView.h"

#include "ObjFile.h"
#include "Morph.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CObjViewView

IMPLEMENT_DYNCREATE(CObjViewView, CView)

BEGIN_MESSAGE_MAP(CObjViewView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_COMMAND(ID_VIEW_ROTATEHORIZONTALLY, &CObjViewView::OnViewRotatehorizontally)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTATEHORIZONTALLY, &CObjViewView::OnUpdateViewRotatehorizontally)
	ON_COMMAND(ID_VIEW_ROTATEVERTICALLY, &CObjViewView::OnViewRotatevertically)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTATEVERTICALLY, &CObjViewView::OnUpdateViewRotatevertically)
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

const GLfloat	white[4]={1.0f,1.0f,1.0f,1.0f}; // RGBA
const GLfloat	palewhite[4]={0.8f,0.8f,0.8f,1.0f}; // RGBA
const GLfloat	lightPosition0[4] = {10.0f,10.0f,20.0f,1.0f};
const GLfloat	lightPosition1[4] = {-10.0f,10.0f,20.0f,1.0f};
const GLfloat	lightAmbient[4]= { 0.2f, 0.2f, 0.2f, 1.0f };

// CObjViewView construction/destruction

CObjViewView::CObjViewView()
: m_hrotating(0)
, m_vrotating(0)
, m_camera_z(10.0f)
{
	m_hrc=NULL;
}

CObjViewView::~CObjViewView()
{
}

BOOL CObjViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CObjViewView message handlers

int CObjViewView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	HWND hWnd = GetSafeHwnd();
	HDC hDC = ::GetDC(hWnd);

#if 0
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;
	
	int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
	TRACE("Pixelformat=%d\n",nPixelFormat);

	BOOL bResult = SetPixelFormat(hDC,nPixelFormat,&pfd);
#endif

	SetDCPixelFormat(hDC, PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);

	m_hrc = wglCreateContext(hDC);
	wglMakeCurrent(hDC,m_hrc);

	return 0;
}

void CObjViewView::OnDestroy()
{
	CView::OnDestroy();

	KillTimer(1);
	wglMakeCurrent(NULL,NULL);
	if (m_hrc) {
		wglDeleteContext(m_hrc);
		m_hrc=NULL;
	}
}

// CObjViewView drawing

void CObjViewView::OnDraw(CDC* pDC)
{
	CObjViewApp *pApp = (CObjViewApp *)AfxGetApp();
	CObjViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	// clear to black
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// save location of camera
	glPushMatrix();

	// if model exists, display it
	if (pDoc->m_cur < pDoc->m_Objs.GetCount()) {

		// save current co-ordinates
		glPushMatrix();

		unsigned long now=pApp->GetTime();
//		TRACE("now=%d etime=%d cur=%d nextcur=%d\n",now,pDoc->m_morph_etime,pDoc->m_cur,pDoc->m_nextcur);
//		if ((now > pDoc->m_morph_etime) && (pDoc->m_cur!=pDoc->m_nextcur)) pDoc->SetModel(pDoc->m_nextcur);

		CObjFile *pObj=pDoc->m_Objs[pDoc->m_cur];

//		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
//		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
//		glEnable(GL_CULL_FACE);

		// object is scaled/shifted to fit in two-unit box about origin
		// rotate
		glRotated(pDoc->m_rotx,1.0,0.0,0.0);
		glRotated(pDoc->m_roty,0.0,1.0,0.0);
		glRotated(pDoc->m_rotz,0.0,0.0,1.0);
		// scale
		glScalef(pObj->m_scale,pObj->m_scale,pObj->m_scale);
		// shift
		glTranslatef(pObj->m_shift[0],pObj->m_shift[1],pObj->m_shift[2]);

//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//		glEnable(GL_LIGHTING);
//		glDisable(GL_CULL_FACE);

		if ((pDoc->m_morph_stime <= now) && (now <= pDoc->m_morph_etime) && (pDoc->m_morph.m_numvertices>0)) {
			pObj->glmDrawMorph(&(pDoc->m_morph),(double)(now-pDoc->m_morph_stime)/(pDoc->m_morph_etime-pDoc->m_morph_stime),GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
		}
		else 
			pObj->glmDraw(GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);

		glPopMatrix();
	}

	// mark the vertices of the two-unit box
	GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat shininess = 65.0f;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glEnable(GL_COLOR_MATERIAL);

	GLUquadricObj *quadric = gluNewQuadric();
    gluQuadricTexture(quadric, GL_TRUE);
    gluQuadricNormals(quadric, GLU_SMOOTH);
	
	glColor3f(0.0f,0.1f,0.5f); 
 
	for (int i=-1;i<=1;i+=2) {
		for (int j=-1;j<=1;j+=2) {
			for (int k=-1;k<=1;k+=2) {
			    glPushMatrix();
		        glTranslated(i,j,k);
		        gluSphere(quadric,0.05,50,50);
			    glPopMatrix(); 
			}
		}
	}


	// restore camera
	glPopMatrix();

	/* that's it */
	glFlush();

	SwapBuffers(pDC->m_hDC);

}

void CObjViewView::SetGLState(int width,int height)
{
	GLdouble	aspect = (GLdouble)width / (GLdouble)height;

	// set up the field of view: angle, aspect, near Clip, far Clip
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, aspect, 1.0, 200.0);

	// set up the camera position
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt (0.0, 0.0, m_camera_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);		// at x=0,y=0,z=10, looking towards origin along -z with +y up

	// set up the lighting
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, palewhite);
	glLightfv(GL_LIGHT0, GL_SPECULAR, palewhite);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, palewhite);
	glLightfv(GL_LIGHT1, GL_SPECULAR, palewhite);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightAmbient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// set up the shading
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);

	// enable the alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CObjViewView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	if ((cx <= 0)|(cy<=0)) return;

	SetGLState(cx,cy);

	// redraw
	Invalidate(FALSE);
}


// CObjViewView printing

BOOL CObjViewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	TRACE("OnPreparePrinting\n");
	return DoPreparePrinting(pInfo);
}

void CObjViewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	TRACE("OnBeginePrinting\n");
}

void CObjViewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	TRACE("OnEndPrinting\n");

	CClientDC dc(this);
	wglMakeCurrent(dc.m_hDC, m_hrc);
	SetDCPixelFormat(dc.m_hDC, PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);

	CRect rcClient;
	GetClientRect(&rcClient); 
	SetGLState(rcClient.Width(),rcClient.Height());
	Invalidate();
}


// CObjViewView diagnostics

#ifdef _DEBUG
void CObjViewView::AssertValid() const
{
	CView::AssertValid();
}

void CObjViewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CObjViewDoc* CObjViewView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CObjViewDoc)));
	return (CObjViewDoc*)m_pDocument;
}
#endif //_DEBUG




BOOL CObjViewView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CObjViewView::OnInitialUpdate()
{
	CClientDC dc(this);
	wglMakeCurrent(dc.m_hDC, m_hrc);

	SetTimer(1,40,0);

	CView::OnInitialUpdate();
}

void CObjViewView::OnViewRotatehorizontally()
{
	m_hrotating=1-m_hrotating;
}

void CObjViewView::OnUpdateViewRotatehorizontally(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_hrotating);
}

void CObjViewView::OnViewRotatevertically()
{
	m_vrotating=1-m_vrotating;
}

void CObjViewView::OnUpdateViewRotatevertically(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_vrotating);
}

void CObjViewView::OnTimer(UINT_PTR nIDEvent)
{
	CObjViewApp *pApp = (CObjViewApp *)AfxGetApp();
	CObjViewDoc* pDoc = GetDocument();

	if (m_hrotating) {
		pDoc->m_roty += 5;
		if (pDoc->m_roty > 360) pDoc->m_roty -= 360;
	}
	if (m_vrotating) {
		pDoc->m_rotx += 5;
		if (pDoc->m_rotx > 360) pDoc->m_rotx -= 360;
	}
	if (m_hrotating || m_vrotating) {
		Invalidate(FALSE);
		UpdateWindow();
	}

	unsigned long now=pApp->GetTime();
	if (now <= pDoc->m_morph_etime) {
		Invalidate(FALSE);
		UpdateWindow();
	}
	else if (pDoc->m_cur!=pDoc->m_nextcur)
		pDoc->SetModel(pDoc->m_nextcur);


	CView::OnTimer(nIDEvent);
}



void CObjViewView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CObjViewDoc* pDoc = GetDocument();

	// TODO: Add your message handler code here and/or call default
//	TRACE("nChar=%d nRepCnt=%d nFlags=%d\n",nChar,nRepCnt,nFlags);
//	TRACE("Shift key = %d\n",GetKeyState(VK_SHIFT));

	int change=5;
	if (GetKeyState(VK_SHIFT)<0) change=1;

	switch (nChar) {
	case 33:
		pDoc->m_rotz -= change;
		Invalidate(FALSE);
		break;
	case 34:
		pDoc->m_rotz += change;
		Invalidate(FALSE);
		break;
	case 37:
		pDoc->m_roty -= change;
		Invalidate(FALSE);
		break;
	case 38:
		pDoc->m_rotx -= change;
		Invalidate(FALSE);
		break;
	case 39:
		pDoc->m_roty += change;
		Invalidate(FALSE);
		break;
	case 40:
		pDoc->m_rotx += change;
		Invalidate(FALSE);
		break;
	}
	if (pDoc->m_roty > 360) pDoc->m_roty -= 360;
	if (pDoc->m_rotx > 360) pDoc->m_rotx -= 360;
	if (pDoc->m_rotz > 360) pDoc->m_rotz -= 360;
	if (pDoc->m_roty < 0) pDoc->m_roty += 360;
	if (pDoc->m_rotx < 0) pDoc->m_rotx += 360;
	if (pDoc->m_rotz < 0) pDoc->m_rotz += 360;

	TRACE("rotation = %g %g %g\n",pDoc->m_rotx,pDoc->m_roty,pDoc->m_rotz);

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CObjViewView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta > 0)
		m_camera_z *= 1.1f;
	else
		m_camera_z *= 1/1.1f;
	if (m_camera_z < 2.0f) m_camera_z=2.0f;		// don't go into display box
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt (0.0, 0.0, m_camera_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);		// at x=0,y=0,z=10, looking towards origin along -z with +y up
	Invalidate(FALSE);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CObjViewView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	CObjViewDoc* pDoc = GetDocument();

	TRACE("OnPrint\n");
	OnPrint1(pDC, pInfo, this);

	// reload the materials
	pDoc->m_Objs.ReloadTextures();
//	CObjFile *pObj=pDoc->m_Objs[pDoc->m_cur];
//	pObj->glmReadMTL(pObj->mtllibname);

	// clear to white
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// save location of camera
	glPushMatrix();

	// if model exists, display it
	if (pDoc->m_cur < pDoc->m_Objs.GetCount()) {

		// save current co-ordinates
		glPushMatrix();

		CObjFile *pObj=pDoc->m_Objs[pDoc->m_cur];

//		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
//		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
//		glEnable(GL_CULL_FACE);

		// object is scaled/shifted to fit in two-unit box about origin
		glRotated(pDoc->m_rotx,1.0,0.0,0.0);
		glRotated(pDoc->m_roty,0.0,1.0,0.0);
		glRotated(pDoc->m_rotz,0.0,0.0,1.0);

//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//		glEnable(GL_LIGHTING);
//		glDisable(GL_CULL_FACE);

		pObj->glmDraw(GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);

		glPopMatrix();
	}

	OnPrint2(pDC);
}

void CObjViewView::OnPrint1(CDC* pDC, CPrintInfo* pInfo, CView* pView) 
{
	// 1. Determine the DIB size for either printing or print preview.
	CRect rcClient;
	pView->GetClientRect(&rcClient); 
	float fClientRatio = float(rcClient.Height())/rcClient.Width();

	// Get page size
	m_szPage.cx  = pDC->GetDeviceCaps(HORZRES);
	m_szPage.cy = pDC->GetDeviceCaps(VERTRES);

	CSize szDIB;
	if (pInfo->m_bPreview)
	{
		// Use screen resolution for preview.
		szDIB.cx = rcClient.Width();
		szDIB.cy = rcClient.Height();
	}
	else  // Printing
	{
		// Use higher resolution for printing.
		// Adjust size according screen's ratio.
		if (m_szPage.cy > fClientRatio*m_szPage.cx)
		{
			// View area is wider than Printer area
			szDIB.cx = m_szPage.cx;
			szDIB.cy = long(fClientRatio*m_szPage.cx);
		}
		else
		{
			// View area is narrower than Printer area
			szDIB.cx = long(float(m_szPage.cy)/fClientRatio);
			szDIB.cy = m_szPage.cy;
		}
		// Reduce the Resolution if the Bitmap size is too big.
		// Ajdust the maximum value, which is depends on printer's memory.
		// I use 20 MB. 
		while (szDIB.cx*szDIB.cy > 20e6)   
		{
			szDIB.cx = szDIB.cx/2;
			szDIB.cy = szDIB.cy/2;
		}
	}

	TRACE("Buffer size: %d x %d = %6.2f MB\n", szDIB.cx, szDIB.cy, szDIB.cx*szDIB.cy*0.000001);
	
	// 2. Create DIB Section
	memset(&m_bmi, 0, sizeof(BITMAPINFO));
	m_bmi.bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	m_bmi.bmiHeader.biWidth			= szDIB.cx;
	m_bmi.bmiHeader.biHeight		= szDIB.cy;
	m_bmi.bmiHeader.biPlanes		= 1;
	m_bmi.bmiHeader.biBitCount		= 24;
	m_bmi.bmiHeader.biCompression	= BI_RGB;
	m_bmi.bmiHeader.biSizeImage		= szDIB.cx * szDIB.cy * 3;

	HDC	hDC = ::GetDC(pView->m_hWnd);
	m_hDib = ::CreateDIBSection(hDC, &m_bmi, DIB_RGB_COLORS, &m_pBitmapBits, NULL, (DWORD)0);
	::ReleaseDC(pView->m_hWnd, hDC);

	// 3. Create memory DC, and associate it with the DIB.
	m_hMemDC = ::CreateCompatibleDC(NULL);
	if (!m_hMemDC)
	{
		DeleteObject(m_hDib);
		m_hDib = NULL;
		return;
	}
	SelectObject(m_hMemDC, m_hDib);

	// 4. Setup memory DC's pixel format.
	if (!SetDCPixelFormat(m_hMemDC, PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL | PFD_STEREO_DONTCARE))
	{
		DeleteObject(m_hDib);
		m_hDib = NULL;
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
		return;
	}

	// 5. Create memory RC
	m_hMemRC = ::wglCreateContext(m_hMemDC);
	if (!m_hMemRC)
	{
		DeleteObject(m_hDib);
		m_hDib = NULL;
		DeleteDC(m_hMemDC);
		m_hMemDC = NULL;
		return;
	}

	// 6. Store old DC and RC
	m_hOldDC = ::wglGetCurrentDC();
	m_hOldRC = ::wglGetCurrentContext(); 

	// 7. Make the memory RC current
	::wglMakeCurrent(m_hMemDC, m_hMemRC);

	// 8. Set OpenGL state for memory RC. 
	// The state is the same as the screen RC's.
	SetGLState(szDIB.cx, szDIB.cy);


}

void CObjViewView::OnPrint2(CDC* pDC) 
{
	// Now the image is in the DIB already. We don't need the memory RC anymore. 
	// We'll copy the image to the DC for printing or print preview.

	// 1. Release memory RC, and restore the old DC and RC.
	::wglMakeCurrent(NULL, NULL);	
	::wglDeleteContext(m_hMemRC);

	// Restore last DC and RC
	::wglMakeCurrent(m_hOldDC, m_hOldRC);	

	// 2. Calculate the target size according to the image size, and orientation of the paper.
	float fBmiRatio = float(m_bmi.bmiHeader.biHeight) / m_bmi.bmiHeader.biWidth;
	CSize szTarget;  
	if (m_szPage.cx > m_szPage.cy)	 // Landscape page
	{
		if(fBmiRatio<1)	  // Landscape image
		{
			szTarget.cx = m_szPage.cx;
			szTarget.cy = long(fBmiRatio * m_szPage.cx);
		}
		else			  // Portrait image
		{
			szTarget.cx = long(m_szPage.cy/fBmiRatio);
			szTarget.cy = m_szPage.cy;
		}
	}
	else		    // Portrait page
	{
		if(fBmiRatio<1)	  // Landscape image
		{
			szTarget.cx = m_szPage.cx;
			szTarget.cy = long(fBmiRatio * m_szPage.cx);
		}
		else			  // Portrait image
		{
			szTarget.cx = long(m_szPage.cy/fBmiRatio);
			szTarget.cy = m_szPage.cy;
		}
	}

	CSize szOffset((m_szPage.cx - szTarget.cx) / 2, (m_szPage.cy - szTarget.cy) / 2);

	// 3. Stretch image to fit in the target size.
	int nRet = ::StretchDIBits(pDC->GetSafeHdc(),
							  szOffset.cx, szOffset.cy,
							  szTarget.cx, szTarget.cy,
							  0, 0,
							  m_bmi.bmiHeader.biWidth, m_bmi.bmiHeader.biHeight,
							  (GLubyte*) m_pBitmapBits,
							  &m_bmi, DIB_RGB_COLORS, SRCCOPY);

	if(nRet == GDI_ERROR) {
		TRACE0("Failed in StretchDIBits()");
	}

	// 4. Release memory.
	DeleteObject(m_hDib);
	m_hDib = NULL;
	DeleteDC(m_hMemDC);	
	m_hMemDC = NULL;
	m_hOldDC = NULL;
}

bool CObjViewView::SetDCPixelFormat(HDC hDC, DWORD dwFlags)
{
	PIXELFORMATDESCRIPTOR pixelDesc;
	
	pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion = 1;
	
	pixelDesc.dwFlags = dwFlags;
	pixelDesc.iPixelType = PFD_TYPE_RGBA;
	pixelDesc.cColorBits = 24;
	pixelDesc.cRedBits = 8;
	pixelDesc.cRedShift = 16;
	pixelDesc.cGreenBits = 8;
	pixelDesc.cGreenShift = 8;
	pixelDesc.cBlueBits = 8;
	pixelDesc.cBlueShift = 0;
	pixelDesc.cAlphaBits = 0;
	pixelDesc.cAlphaShift = 0;
	pixelDesc.cAccumBits = 64;
	pixelDesc.cAccumRedBits = 16;
	pixelDesc.cAccumGreenBits = 16;
	pixelDesc.cAccumBlueBits = 16;
	pixelDesc.cAccumAlphaBits = 0;
	pixelDesc.cDepthBits = 32;
	pixelDesc.cStencilBits = 8;
	pixelDesc.cAuxBuffers = 0;
	pixelDesc.iLayerType = PFD_MAIN_PLANE;
	pixelDesc.bReserved = 0;
	pixelDesc.dwLayerMask = 0;
	pixelDesc.dwVisibleMask = 0;
	pixelDesc.dwDamageMask = 0;
	
	int nPixelIndex = ::ChoosePixelFormat(hDC, &pixelDesc);
	TRACE("Printing Pixelformat=%d\n",nPixelIndex);
	if (nPixelIndex == 0) // Choose default
	{
		nPixelIndex = 1;
		if (::DescribePixelFormat(hDC, nPixelIndex, 
			sizeof(PIXELFORMATDESCRIPTOR), &pixelDesc) == 0)
			return false;
	}

	if (!::SetPixelFormat(hDC, nPixelIndex, &pixelDesc))
		return false;

	return true;
}
