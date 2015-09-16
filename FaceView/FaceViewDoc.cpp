
// FaceViewDoc.cpp : implementation of the CFaceViewDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "FaceView.h"
#endif

#include "MainFrm.h"
#include "FaceViewDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFaceViewDoc

IMPLEMENT_DYNCREATE(CFaceViewDoc, CDocument)

BEGIN_MESSAGE_MAP(CFaceViewDoc, CDocument)
	ON_COMMAND(ID_AVATAR_START, &CFaceViewDoc::OnAvatarStart)
	ON_UPDATE_COMMAND_UI(ID_AVATAR_START, &CFaceViewDoc::OnUpdateAvatarStart)
	ON_COMMAND(ID_AVATAR_STOP, &CFaceViewDoc::OnAvatarStop)
	ON_UPDATE_COMMAND_UI(ID_AVATAR_STOP, &CFaceViewDoc::OnUpdateAvatarStop)
END_MESSAGE_MAP()


// CFaceViewDoc construction/destruction

CFaceViewDoc::CFaceViewDoc()
{
	m_curmorph=m_nextmorph=0;
	m_morph_stime=m_morph_etime=0;
	m_morphing=0;
	m_curlabel="";
}

CFaceViewDoc::~CFaceViewDoc()
{
}

BOOL CFaceViewDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	CMainFrame * pFrame=(CMainFrame *)AfxGetMainWnd();
	if (pFrame) pFrame->Console("OnNewDocument()");

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_Objs.RemoveAll();
	m_Morphs.RemoveAll();
	m_basemesh = NULL;

	return TRUE;
}




// CFaceViewDoc serialization

void CFaceViewDoc::Serialize(CArchive& ar)
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

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CFaceViewDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CFaceViewDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CFaceViewDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CFaceViewDoc diagnostics

#ifdef _DEBUG
void CFaceViewDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFaceViewDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CFaceViewDoc commands


BOOL CFaceViewDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	CMainFrame * pFrame=(CMainFrame *)AfxGetMainWnd();

	CString msg;
	msg.Format("About to open %s",lpszPathName);
	if (pFrame) pFrame->Console((LPCTSTR)msg);
	TRACE(msg);
	TRACE("\n");

	// read the avatar control file, loading models as required
	CString messg;
	CStdioFile ip;
	if (!ip.Open(lpszPathName,CFile::modeRead|CFile::typeText)) {
		messg.Format("Could not find file '%s'",lpszPathName);
		AfxMessageBox(messg,MB_ICONEXCLAMATION);
		return FALSE;
	}
	m_afilename=lpszPathName;

	// look for a folder with the same name in the same location
	char	dirname[256];
	strncpy(dirname,lpszPathName,256);
	char	*p=strrchr(dirname,'.');
	if (p) *p='\0';
	if (SetCurrentDirectory(dirname)==0) {
		ip.Close();
		messg.Format("Could not find folder '%s'",dirname);
		AfxMessageBox(messg,MB_ICONEXCLAMATION);
		return FALSE;
	}

	// look for the names of the models with viseme label
	CString line,viseme,count,model,weight;
	int		i;

	while (ip.ReadString(line)) {
		line.Trim();
TRACE("Got '%s'\n",(LPCTSTR)line);
		if (line.IsEmpty()) continue;
		if (line=="single_model") {
			// format?
		}
		else if (line[0]=='#') {
			// meta command
		}
		else {
			// viseme count model weight
			int valid=0;
			i = line.Find(" ");
			if (i > 0) {
				viseme=line.Left(i);
				line=line.Mid(i+1);
				i = line.Find(" ");
				if (i > 0) {
					count=line.Left(i);
					line=line.Mid(i+1);
					i = line.Find(" ");
					if (i > 0) {
						model=line.Left(i);
						line=line.Mid(i+1);
						line.Trim();
						weight=line;
						TRACE("Viseme: %s %s %s %s\n",(LPCTSTR)viseme,(LPCTSTR)count,(LPCTSTR)model,(LPCTSTR)weight);
						CString msg;
						msg.Format("Loading viseme %s from %s",(LPCTSTR)viseme,(LPCTSTR)model);
						if (pFrame) pFrame->Console((LPCTSTR)msg);
						int idx=m_Objs.Add((LPCTSTR)model,(LPCTSTR)viseme);
						TRACE("Object index = %d\n",idx);
						valid=1;
					}
				}
			}
			if (valid) {
				// load model with viseme tag
			}
			else {
				messg.Format("Could not parse viseme control line in %s",(LPCTSTR)m_afilename);
				AfxMessageBox(messg,MB_ICONEXCLAMATION);
				return FALSE;
			}
		}
	}

	ip.Close();

	// save the base mesh
	m_basemesh = m_Objs.GetAt("x");
	if (m_basemesh==NULL) {
		CString msg;
		msg.Format("Could not find viseme 'x' in '%s'",m_afilename);
		AfxMessageBox((LPCTSTR)msg,MB_ICONEXCLAMATION);
		m_basemesh=m_Objs[0];
	}
	m_curlabel="x";

	// generate all the morphs as differences to base mesh
	m_Morphs.RemoveAll();
	CObjFile	*tstobj;
	CString label;
	for (POSITION pos = m_Objs.m_Labels.GetStartPosition(); pos != NULL;) {
		// one morph per viseme label
		m_Objs.m_Labels.GetNextAssoc(pos, label, (void * &)tstobj);
		CMorph *pMorph = new CMorph();
		pMorph->CreateMorph(m_basemesh,tstobj);
		m_Morphs.Add(pMorph,label);
	}

	UpdateAllViews(NULL);

	return TRUE;
}


void CFaceViewDoc::OnAvatarStart()
{
	m_morphing=1;
	ViewNextMorph();
}


void CFaceViewDoc::OnUpdateAvatarStart(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_morphing==0)&&(m_Morphs.GetCount()>0));
}


void CFaceViewDoc::OnAvatarStop()
{
	m_morphing=0;
}


void CFaceViewDoc::OnUpdateAvatarStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable((m_morphing==1)&&(m_Morphs.GetCount()>0));
}


int CFaceViewDoc::ViewNextMorph(void)
{
	if (m_morphing) {
		CFaceViewApp *pApp = (CFaceViewApp *)AfxGetApp();
		CMainFrame * pFrame=(CMainFrame *)AfxGetMainWnd();
		m_curmorph = m_nextmorph;
		m_nextmorph = (m_nextmorph+1) % m_Morphs.GetCount();
		m_curlabel = m_Morphs.m_Names[m_nextmorph];
		m_morph_stime = pApp->GetTime();
		m_morph_etime = m_morph_stime+2000;
		UpdateAllViews(pFrame->GetConsoleView());
	}

	return 0;
}
