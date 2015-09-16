// ObjView.h : main header file for the ObjView application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CObjViewApp:
// See ObjView.cpp for the implementation of this class
//

class CObjViewApp : public CWinApp
{
public:
	CObjViewApp();

	LARGE_INTEGER	m_hrtfreq;		// frequency of high-resolution timer
	LARGE_INTEGER	m_hrtinit;		// initial value of high-resolution timer

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
	unsigned long GetTime(void);
	int AddFilenameToRecentList(LPCTSTR fname);
};

extern CObjViewApp theApp;