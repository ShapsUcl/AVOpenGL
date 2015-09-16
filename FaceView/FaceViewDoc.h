
// FaceViewDoc.h : interface of the CFaceViewDoc class
//


#pragma once

#include "ObjFile.h"
#include "ObjStore.h"
#include "Morph.h"
#include "MorphStore.h"

class CFaceViewDoc : public CDocument
{
protected: // create from serialization only
	CFaceViewDoc();
	DECLARE_DYNCREATE(CFaceViewDoc)

// Attributes
public:
	CString	m_afilename;			// avatar control file name
	CObjStore	m_Objs;				// array of objects files
	CMorphStore	m_Morphs;			// array of morphs (differences to model 'x')
	CObjFile	*m_basemesh;		// baseline mesh for morphing

	// Morphing demonstration
	int			m_morphing;				// flag indicating morph demonstration in progress
	unsigned long	m_morph_stime;		// start time for current morph
	unsigned long	m_morph_etime;		// end time for current morph
	int			m_curmorph,m_nextmorph;	// current and next morph (indexes into m_Morphs)
	CString		m_curlabel;				// name of current morph

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CFaceViewDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	afx_msg void OnAvatarStart();
	afx_msg void OnUpdateAvatarStart(CCmdUI *pCmdUI);
	afx_msg void OnAvatarStop();
	afx_msg void OnUpdateAvatarStop(CCmdUI *pCmdUI);
	int ViewNextMorph(void);
};
