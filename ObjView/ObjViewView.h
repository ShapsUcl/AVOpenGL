// ObjViewView.h : interface of the CObjViewView class
//


#pragma once

#define DISPLAY_BOX	1

class CObjViewView : public CView
{
protected: // create from serialization only
	CObjViewView();
	DECLARE_DYNCREATE(CObjViewView)

// Attributes
public:
	CObjViewDoc* GetDocument() const;
	HGLRC m_hrc ; 			//OpenGL Rendering Context
	// camera position
	GLfloat	m_camera_z;
	// rotating horizontally
	int m_hrotating;
	// rotating vertically
	int m_vrotating;

	// Printing
	HDC			m_hOldDC;
	HDC			m_hMemDC;
	HGLRC		m_hOldRC;
	HGLRC		m_hMemRC;
	BITMAPINFO	m_bmi;
	LPVOID		m_pBitmapBits;
	HBITMAP		m_hDib;
	CSize		m_szPage;	

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CObjViewView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void OnInitialUpdate();
	afx_msg void OnDestroy();
	afx_msg void OnViewRotatehorizontally();
	afx_msg void OnUpdateViewRotatehorizontally(CCmdUI *pCmdUI);
	afx_msg void OnViewRotatevertically();
	afx_msg void OnUpdateViewRotatevertically(CCmdUI *pCmdUI);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	void OnPrint1(CDC* pDC, CPrintInfo* pInfo, CView* pView);
	void OnPrint2(CDC* pDC);
	bool SetDCPixelFormat(HDC hDC, DWORD dwFlags);
	void SetGLState(int width,int height);
};

#ifndef _DEBUG  // debug version in ObjViewView.cpp
inline CObjViewDoc* CObjViewView::GetDocument() const
   { return reinterpret_cast<CObjViewDoc*>(m_pDocument); }
#endif

