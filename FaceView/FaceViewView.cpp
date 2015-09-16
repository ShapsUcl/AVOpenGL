
// FaceViewView.cpp : implementation of the CFaceViewView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FaceView.h"
#endif

#include "FaceViewDoc.h"
#include "MainFrm.h"
#include "FaceViewView.h"
#include "AudioConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFaceViewView

IMPLEMENT_DYNCREATE(CFaceViewView, CView)

BEGIN_MESSAGE_MAP(CFaceViewView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_COMMAND(ID_AUDIO_CONFIGURE, &CFaceViewView::OnAudioConfigure)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_CONFIGURE, &CFaceViewView::OnUpdateAudioConfigure)
	ON_COMMAND(ID_AUDIO_START, &CFaceViewView::OnAudioStart)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_START, &CFaceViewView::OnUpdateAudioStart)
	ON_COMMAND(ID_AUDIO_PAUSE, &CFaceViewView::OnAudioPause)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_PAUSE, &CFaceViewView::OnUpdateAudioPause)
	ON_COMMAND(ID_AUDIO_STOP, &CFaceViewView::OnAudioStop)
	ON_UPDATE_COMMAND_UI(ID_AUDIO_STOP, &CFaceViewView::OnUpdateAudioStop)
	ON_UPDATE_COMMAND_UI(ID_RECOGNISER_ANNOSOFT, &CFaceViewView::OnUpdateRecogniserAnnosoft)
	ON_UPDATE_COMMAND_UI(ID_GENDER_MALE, &CFaceViewView::OnUpdateGenderMale)
	ON_UPDATE_COMMAND_UI(ID_GENDER_FEMALE, &CFaceViewView::OnUpdateGenderFemale)
	ON_UPDATE_COMMAND_UI(ID_GENDER_GENERIC, &CFaceViewView::OnUpdateGenderGeneric)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_VISEME, &CFaceViewView::OnUpdateViseme)
END_MESSAGE_MAP()

const GLfloat	white[4]={1.0f,1.0f,1.0f,1.0f}; // RGBA
const GLfloat	palewhite[4]={0.8f,0.8f,0.8f,1.0f}; // RGBA
const GLfloat	lightPosition0[4] = {10.0f,10.0f,20.0f,1.0f};
const GLfloat	lightPosition1[4] = {-10.0f,10.0f,20.0f,1.0f};
const GLfloat	lightAmbient[4]= { 0.2f, 0.2f, 0.2f, 1.0f };

// CFaceViewView construction/destruction

CFaceViewView::CFaceViewView()
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();

	// default view
	m_hrc=NULL;
	m_camera_z=10.0f;
	m_rotx=0;
	m_roty=0;
	m_rotz=0;
	m_dragging=0;
	m_rotx_old=0;
	m_roty_old=0;

	// set up audio device
	m_audio.init();
	m_audio_device=-1;
	for (int i=0;i<m_audio.getNumCaptureDevices();i++) {
		char name[256];
		m_audio.getCaptureDeviceName(i,name,256);
		if (pApp->m_microphone.Compare(name)==0) m_audio_device=i;
	}
	m_audio.setCaptureDevice(m_audio_device);
	TRACE("m_audio_device=%d\n",m_audio_device);

	// default state
	m_recording=0;
	m_paused=0;
	m_audio_abort=0;
	m_pmeter=NULL;
	m_totsamp=0;
	m_maxsamp=0;

	m_posmorph=0;
	m_velmorph=0;
	m_lastviseme = "x";
}

CFaceViewView::~CFaceViewView()
{
	if (m_pmeter) delete m_pmeter;
}

BOOL CFaceViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CFaceViewView drawing

void CFaceViewView::OnDraw(CDC* pDC)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	CFaceViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// clear to black
	glClearColor(0.5f, 0.5f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// save location of camera
	glPushMatrix();

	// get current time
	unsigned long now=pApp->GetTime();

	// if model exists, display it
	if ((pDoc->m_basemesh!=NULL) && (pDoc->m_curmorph < pDoc->m_Morphs.GetCount()) && (pDoc->m_nextmorph < pDoc->m_Morphs.GetCount())) {

		// save current co-ordinates
		glPushMatrix();

//		TRACE("now=%d etime=%d cur=%d nextcur=%d\n",now,pDoc->m_morph_etime,pDoc->m_cur,pDoc->m_nextcur);
		if ((now > pDoc->m_morph_etime) && (pDoc->m_curmorph!=pDoc->m_nextmorph)) pDoc->ViewNextMorph();

		CObjFile *pObj=pDoc->m_basemesh;
		m_curmorph.m_numvertices = pObj->numvertices;
		m_curmorph.m_numnormals = pObj->numnormals;
		m_curmorph.m_numtextures = pObj->numtexcoords;

		CMorph *pMorph1=NULL;
		CMorph *pMorph2=NULL;
		double w1=0,w2=0;
		if (m_recording) {
			if (m_vistable.GetCount()==0) {
				// should do nothing
			}
			else if (m_vistable.GetCount()==1) {
				pMorph1=pMorph2=pDoc->m_Morphs.GetAt(m_vistable[0].m_viseme);
				w1=1;
				w2=0;
			}
			else {
				pMorph1=pDoc->m_Morphs.GetAt(m_vistable[0].m_viseme);
				w1=m_vistable[0].m_weight;
				pMorph2=pDoc->m_Morphs.GetAt(m_vistable[1].m_viseme);
				w2=m_vistable[1].m_weight;
			}
		}
		else {
			pMorph1=pDoc->m_Morphs[pDoc->m_curmorph];
			pMorph2=pDoc->m_Morphs[pDoc->m_nextmorph];
			w2=(double)(now-pDoc->m_morph_stime)/(pDoc->m_morph_etime-pDoc->m_morph_stime);
			w1=1-w2;
		}
		CMorph	morph;
		if (pMorph1 && pMorph2)
			morph.CombineMorph(w1,pMorph1,w2,pMorph2);

		// drive current morph towards target
//		m_curmorph.DriveMorph(&morph,m_posmorph,m_velmorph,0.02,(double)(now-m_lastdraw));
		m_curmorph.InterpolateMorph(&morph,0.5);

		// object is scaled/shifted to fit in two-unit box about origin
		// rotate
		glRotated(m_rotx,1.0,0.0,0.0);
		glRotated(m_roty,0.0,1.0,0.0);
		glRotated(m_rotz,0.0,0.0,1.0);
		// scale
		glScalef(pObj->m_scale,pObj->m_scale,pObj->m_scale);
		// shift
		glTranslatef(pObj->m_shift[0],pObj->m_shift[1],pObj->m_shift[2]);

		// draw in-between morph if timing right
		if (m_recording) {
			pObj->glmDrawMorph(&m_curmorph,1.0,GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
		}
		else if ((pDoc->m_morph_stime <= now) && (now <= pDoc->m_morph_etime) && (morph.m_numvertices>0)) {
			pObj->glmDrawMorph(&morph,1.0,GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
		}
		else {
			// draw current model
			pObj->glmDraw(GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
		}

		glPopMatrix();
	}

	// restore camera
	glPopMatrix();

	/* that's it */
	glFlush();

	SwapBuffers(pDC->m_hDC);

	m_lastdraw=now;
}


// CFaceViewView diagnostics

#ifdef _DEBUG
void CFaceViewView::AssertValid() const
{
	CView::AssertValid();
}

void CFaceViewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFaceViewDoc* CFaceViewView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFaceViewDoc)));
	return (CFaceViewDoc*)m_pDocument;
}
#endif //_DEBUG


// CFaceViewView message handlers


int CFaceViewView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// initialise OpenGL on this window
	HWND hWnd = GetSafeHwnd();
	HDC hDC = ::GetDC(hWnd);

	// find the best pixel format
	SetDCPixelFormat(hDC, PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);

	// create OpenGL context and make current
	m_hrc = wglCreateContext(hDC);
	wglMakeCurrent(hDC,m_hrc);

	return 0;
}

// initialise/re-initialise OpenGL window state
void CFaceViewView::SetGLState(int width,int height)
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

bool CFaceViewView::SetDCPixelFormat(HDC hDC, DWORD dwFlags)
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
	TRACE("Pixelformat=%d\n",nPixelIndex);
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



void CFaceViewView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	TRACE("View Onsize(%d,%d)\n",cx,cy);

	if ((cx <= 0)|(cy<=0)) return;

	// re-initialise OpenGL view
	SetGLState(cx,cy);

	// redraw
	Invalidate(FALSE);
}


BOOL CFaceViewView::OnEraseBkgnd(CDC* pDC)
{
//	return CView::OnEraseBkgnd(pDC);
	return TRUE;
}


void CFaceViewView::OnInitialUpdate()
{
	CClientDC dc(this);

	wglMakeCurrent(dc.m_hDC, m_hrc);

	// kick off the main animation timer - runs all the time
	SetTimer(1,20,0);

	CView::OnInitialUpdate();
}


void CFaceViewView::OnDestroy()
{
	CView::OnDestroy();

	// stop the animation timer
	KillTimer(1);

	// close OpenGL context
	wglMakeCurrent(NULL,NULL);
	if (m_hrc) {
		wglDeleteContext(m_hrc);
		m_hrc=NULL;
	}
}

// mouse to rotate model by dragging
void CFaceViewView::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_dragging) {
		m_dragging=0;
		return;
	}
	m_mpoint=point;
	m_dragging=1;
	m_rotx_old = m_rotx;
	m_roty_old = m_roty;
	CView::OnLButtonDown(nFlags, point);
}


void CFaceViewView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_dragging) {
		CRect cr;
		GetClientRect(&cr);
		double xdist = (double)(point.x-m_mpoint.x)/cr.Width();
		double ydist = (double)(point.y-m_mpoint.y)/cr.Height();
		m_rotx = (GLfloat)(m_rotx_old + 180*ydist);
		m_roty = (GLfloat)(m_roty_old + 180*xdist);
		Invalidate(FALSE);
	}
	CView::OnMouseMove(nFlags, point);
}


void CFaceViewView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_dragging) {
		CRect cr;
		GetClientRect(&cr);
		double xdist = (double)(point.x-m_mpoint.x)/cr.Width();
		double ydist = (double)(point.y-m_mpoint.y)/cr.Height();
		m_rotx = (GLfloat)(m_rotx_old + 180*ydist);
		m_roty = (GLfloat)(m_roty_old + 180*xdist);
		Invalidate(FALSE);
	}
	m_dragging=0;
	CView::OnLButtonUp(nFlags, point);
}

// mouse wheel to zoom camera
BOOL CFaceViewView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
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

// main animation timer events
void CFaceViewView::OnTimer(UINT_PTR nIDEvent)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	CMainFrame * pFrame=(CMainFrame *)AfxGetMainWnd();
	CFaceViewDoc* pDoc = GetDocument();

	// morphing between models
	unsigned long now=pApp->GetTime();
	if (now <= pDoc->m_morph_etime) {
		Invalidate(FALSE);
		UpdateWindow();
	}
	else if (pDoc->m_curmorph!=pDoc->m_nextmorph)
		pDoc->ViewNextMorph();

	// audio level meter
	if (m_pmeter) {
		m_pmeter->SetPos((int)(100*m_maxsamp));
		m_maxsamp = (float)(0.95*m_maxsamp);
	}

	// recogniser
	if (m_recording && !m_paused) {
		// copy the contents of the ring buffer to the recogniser
		float	buf[SAMPLE_RATE];
		int	nsamp = m_in_ring.GetCount();
		if (nsamp > SAMPLE_RATE) nsamp=SAMPLE_RATE;
		for (int i=0;i<nsamp;i++) buf[i]=m_in_ring.GetSample();
		pApp->PutVoiceData(buf,nsamp);
		m_totsamp += nsamp;

		// get recognition results
		m_vistable.RemoveAll();
		pApp->GetRecognitionResults((1000*m_totsamp)/SAMPLE_RATE,m_vistable);

		// display results in console
		if (m_vistable.GetCount()>0) {
			CString msg="REC: ";
			int same=0;
			for (int i = 0; i < m_vistable.GetCount(); i++) {
				char	str[256];
				CVisemeMixture m=m_vistable[i];
				sprintf(str,"%s:%.3f ",m.m_viseme,m.m_weight);
				msg.Append(str);
				if (m_lastviseme.Compare(m.m_viseme)==0) same=1;
			}
			if (pFrame) pFrame->Console((LPCTSTR)msg);
			if (!same) {
				m_posmorph=0;
				m_velmorph=0;
				m_lastviseme = m_vistable[0].m_viseme;
			}
			Invalidate(FALSE);
			UpdateWindow();
		}
		else {
			if (pFrame) pFrame->Console("REC: 0");
		}
	}

	CView::OnTimer(nIDEvent);
}

// run audio configuration dialogue
void CFaceViewView::OnAudioConfigure()
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	CAudioConfigDlg	dlg;

	dlg.m_pAudio=&m_audio;
	dlg.m_audio_device=m_audio_device;

	if (dlg.DoModal()==IDOK) {
		m_audio_device = dlg.m_audio_device;
		m_audio.setCaptureDevice(m_audio_device);
		char name[256];
		m_audio.getCaptureDeviceName(m_audio_device,name,256);
		pApp->m_microphone = name;
	}
}


void CFaceViewView::OnUpdateAudioConfigure(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_recording==0);
}

// Audio capture process 
int CaptureCopy(void *context,float *ibuf,int cnt,int nchan)
{
	CFaceViewView *pView= (CFaceViewView *)context;

	/* check if we have stopped */
	if (pView->m_audio_abort) return -1;

	/* copy the data into ring buffer */
	float	max=0;
	for (int i=0;i<cnt*nchan;i++) {
		pView->m_in_ring.PutSample(ibuf[i]);
		if (ibuf[i] > max) max=ibuf[i];
		else if (ibuf[i] < -max) max=-ibuf[i];
	}

	/* keep a record of current status */
	if (max > pView->m_maxsamp) pView->m_maxsamp=max;

	return 0;
}
// start background audio capture process
UINT RunCaptureProcess(LPVOID pParam)
{
	CFaceViewView *pView= (CFaceViewView *)pParam;
	CoInitialize(NULL);
	pView->m_audio.capture(22050,1,220,CaptureCopy,pView);
	CoUninitialize();
	return 0;
}

// start audio capture
void CFaceViewView::OnAudioStart()
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();

	// initialise the recogniser
	pApp->m_annorec.Initialise();

	// create the level meter
	m_pmeter = new CProgressBar("Level:",30,100,1,0);

	// run audio capture in background thread
	m_audio_abort=0;
	m_maxsamp=0;
	m_pCaptureThread=AfxBeginThread(RunCaptureProcess,this,THREAD_PRIORITY_NORMAL);
	m_recording=1;
	m_paused=0;
}


void CFaceViewView::OnUpdateAudioStart(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_recording==0);
}

// pause audio processing (but not audio capture)
void CFaceViewView::OnAudioPause()
{
	m_paused = 1-m_paused;
}


void CFaceViewView::OnUpdateAudioPause(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_recording==1);
	pCmdUI->SetCheck(m_paused);
}

// close down audio capture and recognition
void CFaceViewView::OnAudioStop()
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();

	// close the recogniser
	pApp->m_annorec.Close();

	// close the audio capture
	m_recording=0;
	m_paused=0;
	m_audio_abort=1;
	if (m_pCaptureThread) {
		WaitForSingleObject(m_pCaptureThread->m_hThread, INFINITE); 
		m_pCaptureThread=NULL;
	}

	// delete the level meter
	if (m_pmeter) {
		delete m_pmeter;
		m_pmeter=NULL;
	}
}


void CFaceViewView::OnUpdateAudioStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_recording==1);
}


void CFaceViewView::OnUpdateRecogniserAnnosoft(CCmdUI *pCmdUI)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();

	pCmdUI->Enable(m_recording==0);
	pCmdUI->SetCheck(pApp->m_recogniser==0);
}


void CFaceViewView::OnUpdateGenderMale(CCmdUI *pCmdUI)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	pCmdUI->Enable(m_recording==0);
	pCmdUI->SetCheck(pApp->m_gender==0);
}


void CFaceViewView::OnUpdateGenderFemale(CCmdUI *pCmdUI)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	pCmdUI->Enable(m_recording==0);
	pCmdUI->SetCheck(pApp->m_gender==1);
}


void CFaceViewView::OnUpdateGenderGeneric(CCmdUI *pCmdUI)
{
	CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
	pCmdUI->Enable(m_recording==0);
	pCmdUI->SetCheck(pApp->m_gender==2);
}

void CFaceViewView::OnUpdateViseme(CCmdUI *pCmdUI)
{
	CFaceViewDoc* pDoc = GetDocument();
	pCmdUI->Enable();
	CString txt="";
	if (m_recording) {
		if (m_vistable.GetCount()>0)
			txt = m_vistable[0].m_viseme;
	}
	else
		txt=pDoc->m_curlabel;
	pCmdUI->SetText(txt);
}
