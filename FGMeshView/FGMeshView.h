
// FGMeshView.h : main header file for the FGMeshView application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

//extern struct FgGuiWinMain;

// CFGMeshViewApp:
// See FGMeshView.cpp for the implementation of this class
//
//FgGuiWinStatics s_fgGuiWin;


class CFGMeshViewApp : public CWinAppEx
{
public:
	CFGMeshViewApp();


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};
//extern struct  FgGuiWinTabs;
extern CFGMeshViewApp theApp;
