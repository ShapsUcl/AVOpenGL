// ObjViewDoc.h : interface of the CObjViewDoc class
//


#pragma once

#include "ObjFile.h"
#include "ObjStore.h"
#include "Morph.h"

class CObjViewDoc : public CDocument
{
protected: // create from serialization only
	CObjViewDoc();
	DECLARE_DYNCREATE(CObjViewDoc)

// Attributes
public:
	CObjStore	m_Objs;
	int			m_cur;
	GLfloat		m_origscale;
	GLfloat		m_smoothing_angle;
	GLdouble m_roty;
	GLdouble m_rotx;
	GLdouble m_rotz;

// Morphing
	CMorph		m_morph;
	unsigned long	m_morph_stime;
	unsigned long	m_morph_etime;
	int			m_nextcur;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CObjViewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	afx_msg void OnUpdateFileSave(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI *pCmdUI);
	afx_msg void OnFileOpen();
	afx_msg void OnViewNextmodel();
	afx_msg void OnViewPreviousmodel();
	afx_msg void OnUpdateViewNextmodel(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewPreviousmodel(CCmdUI *pCmdUI);
	afx_msg void OnViewResetposition();
	afx_msg void OnUpdateViewResetposition(CCmdUI *pCmdUI);
	afx_msg void OnViewNextmorph();
	afx_msg void OnUpdateViewNextmorph(CCmdUI *pCmdUI);
	afx_msg void OnViewPreviousmorph();
	afx_msg void OnUpdateViewPreviousmorph(CCmdUI *pCmdUI);
	void SetModel(int idx);
};


