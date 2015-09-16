// ObjViewDoc.cpp : implementation of the CObjViewDoc class
//

#include "stdafx.h"
#include "ObjView.h"

#include "ObjViewDoc.h"

#include "Shlwapi.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CObjViewDoc

IMPLEMENT_DYNCREATE(CObjViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CObjViewDoc, CDocument)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &CObjViewDoc::OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &CObjViewDoc::OnUpdateFileSaveAs)
	ON_COMMAND(ID_FILE_OPEN, &CObjViewDoc::OnFileOpen)
	ON_COMMAND(ID_VIEW_NEXTMODEL, &CObjViewDoc::OnViewNextmodel)
	ON_COMMAND(ID_VIEW_PREVIOUSMODEL, &CObjViewDoc::OnViewPreviousmodel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTMODEL, &CObjViewDoc::OnUpdateViewNextmodel)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIOUSMODEL, &CObjViewDoc::OnUpdateViewPreviousmodel)
	ON_COMMAND(ID_VIEW_RESETPOSITION, &CObjViewDoc::OnViewResetposition)
	ON_UPDATE_COMMAND_UI(ID_VIEW_RESETPOSITION, &CObjViewDoc::OnUpdateViewResetposition)
	ON_COMMAND(ID_VIEW_NEXTMORPH, &CObjViewDoc::OnViewNextmorph)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NEXTMORPH, &CObjViewDoc::OnUpdateViewNextmorph)
	ON_COMMAND(ID_VIEW_PREVIOUSMORPH, &CObjViewDoc::OnViewPreviousmorph)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PREVIOUSMORPH, &CObjViewDoc::OnUpdateViewPreviousmorph)
END_MESSAGE_MAP()


// CObjViewDoc construction/destruction

CObjViewDoc::CObjViewDoc()
{
	m_origscale=1.0;
	m_smoothing_angle=90.0;
	m_rotx = m_roty = m_rotz = 0;
	m_cur=m_nextcur=0;
	m_morph_stime=m_morph_etime=0;
}

CObjViewDoc::~CObjViewDoc()
{
}

BOOL CObjViewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	TRACE("OnNewDocument()\n");

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_Objs.RemoveAll();
	m_rotx = m_roty = m_rotz = 0;

	UpdateAllViews(NULL);

	return TRUE;
}


// CObjViewDoc serialization

void CObjViewDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CObjViewDoc diagnostics

#ifdef _DEBUG
void CObjViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CObjViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CObjViewDoc commands

BOOL CObjViewDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	TRACE("OnOpenDocument(%s)\n",lpszPathName);

	// load in the OBJ format file
	if (m_Objs.Add(lpszPathName)!=0) {
		return FALSE;
	}

	m_cur=m_Objs.GetCount()-1;

	CString fname=m_Objs[m_cur]->pathname;
	::PathStripPath(fname.GetBuffer());
	fname.Replace(".obj","");
	TRACE("SetTitle %s\n",(LPCTSTR)fname);
	SetTitle((LPCTSTR)fname);
	fname.ReleaseBuffer(-1);

	UpdateAllViews(NULL);

	return TRUE;
}

BOOL CObjViewDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	if (m_Objs.GetCount()!=1) return FALSE;
	CObjFile *pObj = m_Objs[0];

	/* rotate actual model to displayed position */
	pObj->glmRotate((GLfloat)m_rotx,(GLfloat)m_roty,(GLfloat)m_rotz);
	m_rotx=m_roty=m_rotz=0;

	pObj->glmFacetNormals();
	pObj->glmVertexNormals(m_smoothing_angle);

	pObj->pathname = lpszPathName;
	pObj->mtllibname = lpszPathName;
	::PathStripPath(pObj->mtllibname.GetBuffer());
	pObj->mtllibname.ReleaseBuffer(-1);
	if (pObj->mtllibname.Find(".obj")>0)
		pObj->mtllibname.Replace(".obj",".mtl");
	else
		pObj->mtllibname.Append(".mtl");
	
	if (pObj->glmWriteOBJ(lpszPathName,GLM_SMOOTH|GLM_MATERIAL)==0) {
		CString msg;
		msg.Format("Failed to save model to '%s'",lpszPathName);
		AfxMessageBox((LPCTSTR)msg,MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
	}

	UpdateAllViews(NULL);
	return TRUE;
}

void CObjViewDoc::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_Objs.GetCount()==1);
}

void CObjViewDoc::OnUpdateFileSaveAs(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_Objs.GetCount()==1);
}


void CObjViewDoc::OnFileOpen()
{
	CObjViewApp *pApp = (CObjViewApp *)AfxGetApp();
	TCHAR FileNameList[8192]= {0};

	char strFilter[] = { "Wavefront 3D model (*.obj)|*.obj*||" };

    CFileDialog FileDlg(TRUE, "obj", NULL, OFN_ALLOWMULTISELECT, strFilter);
	FileDlg.m_ofn.lpstrFile = FileNameList;
	FileDlg.m_ofn.nMaxFile=8192;
	if( FileDlg.DoModal() == IDOK ) {
		CWaitCursor cw;
		CString csFname;
		POSITION pos = FileDlg.GetStartPosition();
		while (pos != NULL) {
			csFname = FileDlg.GetNextPathName(pos);
			TRACE("%s\n", csFname);
			if (!OnOpenDocument((LPCTSTR)csFname)) return;
			pApp->AddFilenameToRecentList((LPCTSTR)csFname);
		}
    }
    else
    	return;	
}


void CObjViewDoc::OnViewNextmodel()
{
	if (m_cur < m_Objs.GetCount()-1) m_cur++;
	CString fname=m_Objs[m_cur]->pathname;
	::PathStripPath(fname.GetBuffer());
	fname.Replace(".obj","");
	TRACE("SetTitle %s\n",(LPCTSTR)fname);
	SetTitle((LPCTSTR)fname);
	fname.ReleaseBuffer(-1);
	m_nextcur=m_cur;
	UpdateAllViews(NULL);
}


void CObjViewDoc::OnViewPreviousmodel()
{
	if (m_cur > 0) m_cur--;
	CString fname=m_Objs[m_cur]->pathname;
	::PathStripPath(fname.GetBuffer());
	fname.Replace(".obj","");
	TRACE("SetTitle %s\n",(LPCTSTR)fname);
	SetTitle((LPCTSTR)fname);
	fname.ReleaseBuffer(-1);
	m_nextcur=m_cur;
	UpdateAllViews(NULL);
}

void CObjViewDoc::SetModel(int idx)
{
	TRACE("SetModel(%d)\n",idx);
	if ((0<=idx)&&(idx<m_Objs.GetCount())) {
		m_cur=idx;
		CString fname=m_Objs[m_cur]->pathname;
		::PathStripPath(fname.GetBuffer());
		fname.Replace(".obj","");
		TRACE("SetTitle %s\n",(LPCTSTR)fname);
		SetTitle((LPCTSTR)fname);
		fname.ReleaseBuffer(-1);
		m_nextcur=m_cur;
		UpdateAllViews(NULL);
	}
}


void CObjViewDoc::OnUpdateViewNextmodel(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_Objs.GetCount()>1)&&(m_cur<m_Objs.GetCount()-1));
}


void CObjViewDoc::OnUpdateViewPreviousmodel(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_Objs.GetCount()>1)&&(m_cur>0));
}


void CObjViewDoc::OnViewResetposition()
{
	m_roty=0;
	m_rotx=0;
	m_rotz=0;
	UpdateAllViews(NULL);
}


void CObjViewDoc::OnUpdateViewResetposition(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_Objs.GetCount()>0);
}


void CObjViewDoc::OnViewNextmorph()
{
	CObjViewApp *pApp = (CObjViewApp *)AfxGetApp();
	if (m_cur < m_Objs.GetCount()-1) {
		m_morph.CreateMorph(m_Objs[m_cur],m_Objs[m_cur+1]);
		m_morph_stime = pApp->GetTime();
		m_morph_etime = m_morph_stime+2000;
		m_nextcur=m_cur+1;
		UpdateAllViews(NULL);
	}
}

void CObjViewDoc::OnUpdateViewNextmorph(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_Objs.GetCount()>1)&&(m_cur<m_Objs.GetCount()-1));
}

void CObjViewDoc::OnViewPreviousmorph()
{
	CObjViewApp *pApp = (CObjViewApp *)AfxGetApp();
	if (m_cur > 0) {
		m_morph.CreateMorph(m_Objs[m_cur],m_Objs[m_cur-1]);
		m_morph_stime = pApp->GetTime();
		m_morph_etime = m_morph_stime+2000;
		m_nextcur=m_cur-1;
		UpdateAllViews(NULL);
	}
}

void CObjViewDoc::OnUpdateViewPreviousmorph(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_Objs.GetCount()>1)&&(m_cur>0));
}
