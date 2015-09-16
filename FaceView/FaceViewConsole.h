
// FaceViewView.h : interface of the CFaceViewView class
//

#pragma once


class CFaceViewConsole : public CView
{
protected: // create from serialization only
	CFaceViewConsole();
	DECLARE_DYNCREATE(CFaceViewConsole)

// Attributes
public:
	CFaceViewDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CFaceViewConsole();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CListBox	*m_plist;
	CBrush		m_bgbrush;
	CFont		m_font;
	int			m_maxstring;

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	void AddString(LPCTSTR str);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // debug version in FaceViewView.cpp
inline CFaceViewDoc* CFaceViewConsole::GetDocument() const
   { return reinterpret_cast<CFaceViewDoc*>(m_pDocument); }
#endif

