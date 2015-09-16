
// FaceView.h : main header file for the FaceView application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include "AnnoSoftRec.h"

// CFaceViewApp:
// See FaceView.cpp for the implementation of this class
//

class CFaceViewApp : public CWinApp
{
public:
	CFaceViewApp();

	LARGE_INTEGER	m_hrtfreq;		// frequency of high-resolution timer
	LARGE_INTEGER	m_hrtinit;		// initial value of high-resolution timer

	// directories
	CString			m_datadir;

	// Annosoft recogniser
	CAnnoSoftRec	m_annorec;
	int				m_recogniser;
	int				m_gender;

	// Configuration
	CString			m_microphone;

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	unsigned long GetTime(void);
	int GetRecognitionResults(long mstime,CArray<CVisemeMixture,CVisemeMixture &> &list);
	int PutVoiceData(float *buf,int cnt);

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnRecogniserAnnosoft();
	afx_msg void OnGenderMale();
	afx_msg void OnGenderFemale();
	afx_msg void OnGenderGeneric();
};

extern CFaceViewApp theApp;
