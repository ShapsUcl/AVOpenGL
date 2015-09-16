
// FaceViewView.h : interface of the CFaceViewView class
//

#pragma once

#include "CoreAudio.h"
#include "RingBuffer.h"
#include "ProgressBar.h"
#include "VisemeMixture.h"

class CFaceViewView : public CView
{
protected: // create from serialization only
	CFaceViewView();
	DECLARE_DYNCREATE(CFaceViewView)

// Attributes
public:
	CFaceViewDoc* GetDocument() const;
	HGLRC m_hrc ; 			//OpenGL Rendering Context

	// camera position
	GLfloat	m_camera_z;

	// model rotation
	GLfloat m_roty;
	GLfloat m_rotx;
	GLfloat m_rotz;

	// mouse drag point
	CPoint	m_mpoint;
	GLfloat	m_rotx_old;
	GLfloat	m_roty_old;
	int		m_dragging;

	// audio capture
	CCoreAudio	m_audio;
	int		m_audio_device;
	int		m_recording;
	int		m_paused;
	int		m_audio_abort;
	CWinThread	*m_pCaptureThread;
	CProgressBar	*m_pmeter;
	float		m_maxsamp;
	CRingBuffer	m_in_ring;				// incoming ring buffer
	int		m_totsamp;					// total # samples sent to recogniser

	// recognition result
	CArray<CVisemeMixture,CVisemeMixture &>	m_vistable;
	CString	m_lastviseme;
	CMorph	m_curmorph;					// current morph to base mesh
	double	m_posmorph;					// current interpolation
	double	m_velmorph;					// current velocity
	unsigned long m_lastdraw;			// last draw time

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CFaceViewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	bool SetDCPixelFormat(HDC hDC, DWORD dwFlags);
	void SetGLState(int width,int height);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnAudioConfigure();
	afx_msg void OnUpdateAudioConfigure(CCmdUI *pCmdUI);
	afx_msg void OnAudioStart();
	afx_msg void OnUpdateAudioStart(CCmdUI *pCmdUI);
	afx_msg void OnAudioPause();
	afx_msg void OnUpdateAudioPause(CCmdUI *pCmdUI);
	afx_msg void OnAudioStop();
	afx_msg void OnUpdateAudioStop(CCmdUI *pCmdUI);
	afx_msg void OnUpdateRecogniserAnnosoft(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGenderMale(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGenderFemale(CCmdUI *pCmdUI);
	afx_msg void OnUpdateGenderGeneric(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViseme(CCmdUI *pCmdUI);
};

#ifndef _DEBUG  // debug version in FaceViewView.cpp
inline CFaceViewDoc* CFaceViewView::GetDocument() const
   { return reinterpret_cast<CFaceViewDoc*>(m_pDocument); }
#endif

