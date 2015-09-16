
// FGMeshViewView.h : interface of the CFGMeshViewView class
//
#include <FgMain.hpp>
#include <FgCommand.hpp>
#include "FgMatrixC.hpp"
#include "FgGuiApiBase.hpp"
#include "FgGuiWin.hpp"
#include "FgThrowWindows.hpp"
#include "FgMetaFormat.hpp"
#include "FgHex.hpp"
#include "FGMeshViewDoc.h"

#pragma once

extern void viewMesh(const FgArgs & args);	// defined in FgCmdView.hpp

FgCmd
fgCmdViewMesh3D(const char * args);

class CFGMeshViewView : public CView
{
protected: // create from serialization only
	CFGMeshViewView();
	DECLARE_DYNCREATE(CFGMeshViewView)

// Attributes
public:
	CFGMeshViewDoc* GetDocument() const;
	CFGMeshViewView * GetCFGMeshView(); // { return (CFGMeshViewView *) pMainWnd; };
	int m_init;
	HGLRC m_hrc ; 			//OpenGL Rendering Context

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
	virtual ~CFGMeshViewView();
	void CFGMeshViewView::fgGuiImplStart(
		const FgString &            title,
		FgSharedPtr<FgGuiApiBase>   def,
		const FgString &            storeDir,
		const FgGuiOptions &        opts);
	int InitGui(void); 

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);	
	afx_msg void OnPaint();	
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in FGMeshViewView.cpp
inline CFGMeshViewDoc* CFGMeshViewView::GetDocument() const
   { return reinterpret_cast<CFGMeshViewDoc*>(m_pDocument); }
#endif

