
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include "FGMeshViewDoc.h"
#include "FGMeshViewView.h"

class CMainFrame : public CFrameWndEx
{
	
protected: // create from serialization only
	CMainFrame();
	CSplitterWnd m_wndSplitter;
	int			m_wndSplitterInit;
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
//	FgGuiWinMain FgGui;

// Operations
public:
//	CFGMeshViewView * GetCFGMeshView() { return (CMainFrame *) pApp->m_pMainWnd; };

	// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//virtual BOOL OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	//	CCreateContext* pContext);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar       m_wndMenuBar;
	CMFCToolBar       m_wndToolBar;
	CMFCStatusBar     m_wndStatusBar;
	CMFCToolBarImages m_UserImages;

// Generated message map functions
protected:
	//afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	CFGMeshViewView * GetMeshView(void);
};


